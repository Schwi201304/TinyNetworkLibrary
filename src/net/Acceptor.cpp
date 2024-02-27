#include "net/Acceptor.hpp"
#include "base/base.hpp"
#include "net/InetAddress.hpp"

#include <functional>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>

namespace schwi
{
    static int createNonblocking()
    {
        int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (sockfd < 0)
        {
            LOG_ERROR("listen socket create error {}", strerror(errno));
            abort();
        }
        return sockfd;
    }

    Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
        : _loop(loop),
          _acceptSocket(createNonblocking()),
          _acceptChannel(loop, _acceptSocket.fd()),
          _listenning(false)
    {
        _acceptSocket.setReuseAddr(true);
        _acceptSocket.setReusePort(reuseport);
        _acceptSocket.bindAddress(listenAddr);
        _acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
    }

    Acceptor::~Acceptor()
    {
        _acceptChannel.disableAll();
        _acceptChannel.remove();
    }

    void Acceptor::listen()
    {
        _listenning = true;
        _acceptSocket.listen();
        _acceptChannel.enableReading();
    }

    void Acceptor::handleRead()
    {
        InetAddress peerAddr(0);
        int connfd = _acceptSocket.accept(&peerAddr);
        if (connfd >= 0)
        {
            if (_newConnectionCallback)
            {
                _newConnectionCallback(connfd, peerAddr);
            }
            else
            {
                LOG_ERROR("no newConnectionCallback() in Acceptor::handleRead");
                ::close(connfd);
            }
        }
        else
        {
            LOG_ERROR("accept() failed in Acceptor::handleRead");
            if (errno == EMFILE)
            {
                LOG_ERROR("EMFILE error");
            }
        }
    }
} // namespace schwi
