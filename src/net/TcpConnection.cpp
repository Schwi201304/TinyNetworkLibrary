#include "net/TcpConnection.hpp"
#include "net/Channel.hpp"
#include "net/EventLoop.hpp"
#include "net/Socket.hpp"
#include "base/base.hpp"

#include <functional>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>

namespace schwi
{
    static EventLoop *checkLoopNotNull(EventLoop *loop)
    {
        if (loop == nullptr)
        {
            LOG_FATAL("TcpConnection::checkLoopNotNull - EventLoop is null");
        }
        return loop;
    }

    TcpConnection::TcpConnection(EventLoop *loop,
                                 const std::string &name,
                                 int sockfd,
                                 const InetAddress &localAddr,
                                 const InetAddress &peerAddr)
        : _loop(checkLoopNotNull(loop)),
          _name(name),
          _state(kConnecting),
          _reading(true),
          _socket(new Socket(sockfd)),
          _channel(new Channel(loop, sockfd)),
          _localAddr(localAddr),
          _peerAddr(peerAddr),
          _highWaterMark(64 * 1024 * 1024)
    {
        _channel->setReadCallback(
            std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
        _channel->setWriteCallback(
            std::bind(&TcpConnection::handleWrite, this));
        _channel->setCloseCallback(
            std::bind(&TcpConnection::handleClose, this));
        _channel->setErrorCallback(
            std::bind(&TcpConnection::handleError, this));
        LOG_DEBUG("TcpConnection::ctor[{}] at {} fd={}",
                  _name.c_str(), this, sockfd);
        _socket->setKeepAlive(true);
    }

    TcpConnection::~TcpConnection()
    {
        LOG_DEBUG("TcpConnection::dtor[{}] at {} fd={}",
                  _name.c_str(), this, _channel->fd());
    }

    void TcpConnection::send(const std::string &message)
    {
        if (_state == kConnected)
        {
            if (_loop->isInLoopThread())
            {
                sendInLoop(message);
            }
            else
            {
                void (TcpConnection::*fp)(const std::string &message) = &TcpConnection::sendInLoop;
                _loop->runInLoop(
                    std::bind(fp, this, message));
            }
        }
    }

    void TcpConnection::send(Buffer *message)
    {
        if (_state == kConnected)
        {
            if (_loop->isInLoopThread())
            {
                sendInLoop(message->peek(), message->readableBytes());
                message->retrieveAll();
            }
            else
            {
                void (TcpConnection::*fp)(const std::string &message) = &TcpConnection::sendInLoop;
                _loop->runInLoop(
                    std::bind(fp,
                              this,
                              message->retrieveAllAsString()));
            }
        }
    }

    void TcpConnection::sendInLoop(const std::string &message)
    {
        sendInLoop(message.data(), message.size());
    }

    void TcpConnection::sendInLoop(const void *message, size_t len)
    {
        ssize_t nwrote = 0;
        size_t remaining = len;
        bool error = false;

        if (_state == kDisconnected)
        {
            LOG_ERROR("disconnected, give up writing");
            return;
        }
        // if no thing in output queue, try writing directly
        if (!_channel->isWriting() && _outputBuffer.readableBytes() == 0)
        {
            nwrote = ::write(_channel->fd(), message, len);
            if (nwrote >= 0)
            {
                remaining = len - nwrote;
                if (remaining == 0 && _writeCompleteCallback)
                {
                    _loop->queueInLoop(
                        std::bind(_writeCompleteCallback, shared_from_this()));
                }
            }
            else // nwrote < 0
            {
                nwrote = 0;
                if (errno != EWOULDBLOCK)
                {
                    LOG_ERROR("TcpConnection::sendInLoop");
                    if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                    {
                        error = true;
                    }
                }
            }
        }

        if (!error && remaining > 0)
        {
            size_t oldLen = _outputBuffer.readableBytes();
            if (oldLen + remaining >= _highWaterMark &&
                oldLen < _highWaterMark &&
                _highWaterMarkCallback)
            {
                _loop->queueInLoop(
                    std::bind(_highWaterMarkCallback,
                              shared_from_this(),
                              oldLen + remaining));
            }
            _outputBuffer.append(static_cast<const char *>(message) + nwrote, remaining);
            if (!_channel->isWriting())
            {
                _channel->enableWriting();
            }
        }
    }

    void TcpConnection::shutdown()
    {
        if (_state == kConnected)
        {
            setState(kDisconnecting);
            _loop->runInLoop(
                std::bind(&TcpConnection::shutdownInLoop, this));
        }
    }

    void TcpConnection::shutdownInLoop()
    {
        if (!_channel->isWriting())
        {
            _socket->shutdownWrite();
        }
    }

    void TcpConnection::connectEstablished()
    {
        setState(kConnected);
        _channel->tie(shared_from_this());
        _channel->enableReading();

        _connectionCallback(shared_from_this());
    }

    void TcpConnection::connectDestroyed()
    {
        if (_state == kConnected)
        {
            setState(kDisconnected);
            _channel->disableAll();

            _connectionCallback(shared_from_this());
        }
        _channel->remove();
    }

    void TcpConnection::handleRead(Timestamp receiveTime)
    {
        int savedErrno = 0;
        ssize_t n = _inputBuffer.readFd(_channel->fd(), &savedErrno);
        if (n > 0)
        {
            _messageCallback(shared_from_this(), &_inputBuffer, receiveTime);
        }
        else if (n == 0)
        {
            handleClose();
        }
        else
        {
            errno = savedErrno;
            LOG_ERROR("TcpConnection::handleRead");
            handleError();
        }
    }

    void TcpConnection::handleWrite()
    {
        if (_channel->isWriting())
        {
            int saveErrno = 0;
            ssize_t n = _outputBuffer.writeFd(_channel->fd(), &saveErrno);
            if (n > 0)
            {
                _outputBuffer.retrieve(n);
                if (_outputBuffer.readableBytes() == 0)
                {
                    _channel->disableWriting();
                    if (_writeCompleteCallback)
                    {
                        _loop->queueInLoop(
                            std::bind(_writeCompleteCallback, shared_from_this()));
                    }
                    if (_state == kDisconnecting)
                    {
                        shutdownInLoop();
                    }
                }
            }
            else
            {
                LOG_ERROR("TcpConnection::handleWrite");
            }
        }
        else
        {
            LOG_ERROR("Connection fd = {} is down, no more writing", _channel->fd());
        }
    }

    void TcpConnection::handleClose()
    {
        LOG_DEBUG("fd = {} state = {}", _channel->fd(), _state.load());
        setState(kDisconnected);
        _channel->disableAll();

        TcpConnectionPtr guardThis(shared_from_this());
        _connectionCallback(guardThis);
        _closeCallback(guardThis);
    }

    void TcpConnection::handleError()
    {
        int optval;
        socklen_t optlen = sizeof(optval);
        int err = 0;
        if (::getsockopt(_channel->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen))
        {
            err = errno;
        }
        else
        {
            err = optval;
        }
        LOG_ERROR("TcpConnection::handleError [{}] - SO_ERROR = {} {}", _name.c_str(), err, strerror(err));
    }
} // namespace schwi
