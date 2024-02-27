#include "net/TcpServer.hpp"
#include "net/TcpConnection.hpp"
#include "base/base.hpp"

namespace schwi
{
    static EventLoop *CheckLoopNotNull(EventLoop *loop)
    {
        if (loop == nullptr)
        {
            LOG_FATAL("EventLoop is null");
        }
        return loop;
    }

    TcpServer::TcpServer(EventLoop *loop,
                         const InetAddress &listenAddr,
                         const std::string &name,
                         Option option)
        : _loop(CheckLoopNotNull(loop)),
          _ipPort(listenAddr.toIpPort()),
          _name(name),
          _acceptor(new Acceptor(loop, listenAddr, option == kReusePort)),
          _threadPool(new EventLoopThreadPool(loop, name)),
          _connectionCallback(),
          _messageCallback(),
          _writeCompleteCallback(),
          _threadInitCallback(),
          _started(0),
          _nextConnId(1)
    {
        _acceptor->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
    }

    TcpServer::~TcpServer()
    {
        LOG_TRACE("TcpServer::~TcpServer [{}] destructing", _name);

        for (auto &item : _connections)
        {
            TcpConnectionPtr conn(item.second);
            item.second.reset();
            conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectDestroyed, conn));
        }
    }

    void TcpServer::setThreadNum(int numThreads)
    {
        _threadPool->setThreadNum(numThreads);
    }

    void TcpServer::start()
    {
        LOG_DEBUG("TcpServer::start() _started = {}", _started.load());
        if (_started++ == 0)
        {
            _threadPool->start(_threadInitCallback);

            _loop->runInLoop(
                std::bind(&Acceptor::listen, _acceptor.get()));
        }
    }

    void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
    {
        EventLoop *ioLoop = _threadPool->getNextLoop();
        char buf[64];
        snprintf(buf, sizeof buf, "-%s#%d", _ipPort.c_str(), _nextConnId);
        ++_nextConnId;
        std::string connName = _name + buf;

        LOG_INFO("TcpServer::newConnection [{}] - new connection [{}] from {}",
                 _name, connName, peerAddr.toIpPort());

        sockaddr_in local;
        bzero(&local, sizeof local);
        socklen_t addrlen = sizeof local;
        if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0)
        {
            LOG_ERROR("sockets::getLocalAddr");
        }

        InetAddress localAddr(local);
        TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                                connName,
                                                sockfd,
                                                localAddr,
                                                peerAddr));

        _connections[connName] = conn;
        conn->setConnectionCallback(_connectionCallback);
        conn->setMessageCallback(_messageCallback);
        conn->setWriteCompleteCallback(_writeCompleteCallback);
        conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
        ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    }

    void TcpServer::removeConnection(const TcpConnectionPtr &conn)
    {
        _loop->runInLoop(
            std::bind(&TcpServer::removeConnectionInLoop, this, conn));
    }

    void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
    {
        LOG_INFO("TcpServer::removeConnectionInLoop [{}] - connection {}", _name, conn->name());

        _connections.erase(conn->name());
        EventLoop *ioLoop = conn->getLoop();
        ioLoop->queueInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
} // namespace schwi
