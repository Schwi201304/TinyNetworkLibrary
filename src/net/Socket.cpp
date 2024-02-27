#include "net/Socket.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <errno.h>

#include "base/base.hpp"
#include "net/InetAddress.hpp"

namespace schwi
{
    /**
     * @brief 析构函数
     */
    Socket::~Socket()
    {
        ::close(_sockfd);
    }

    /**
     * @brief 绑定地址
     * @param localaddr 地址
     */
    void Socket::bindAddress(const InetAddress &localaddr)
    {
        if (::bind(_sockfd, (sockaddr *)localaddr.getSockAddr(), sizeof(struct sockaddr_in)) != 0)
        {
            LOG_FATAL("bind socket:{} failed, error:{}", _sockfd, strerror(errno));
        }
    }

    /**
     * @brief 监听
     */
    void Socket::listen()
    {
        if (::listen(_sockfd, SOMAXCONN) != 0)
        {
            LOG_FATAL("listen socket:{} failed", _sockfd);
        }
    }

    /**
     * @brief 接受连接
     * @param peeraddr 对端地址
     * @return int 文件描述符
     */
    int Socket::accept(InetAddress *peeraddr)
    {
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        ::memset(&addr, 0, addrlen);

        int connfd = ::accept4(_sockfd, (sockaddr *)&addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (connfd < 0)
        {
            LOG_ERROR("accept socket:{} failed", _sockfd);
        }
        peeraddr->setSockAddr(addr);
        return connfd;
    }

    /**
     * @brief 关闭写端
     */
    void Socket::shutdownWrite()
    {
        if (::shutdown(_sockfd, SHUT_WR) != 0)
        {
            LOG_ERROR("shutdown socket:{} failed", _sockfd);
        }
    }

    /**
     * @brief 设置TCP_NODELAY
     * @param on 是否开启
     */
    void Socket::setTcpNoDelay(bool on)
    {
        int optval = on ? 1 : 0;
        if (::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval))) != 0)
        {
            LOG_ERROR("setsockopt TCP_NODELAY socket:{} failed", _sockfd);
        }
    }

    /**
     * @brief 设置地址复用
     * @param on 是否复用
     */
    void Socket::setReuseAddr(bool on)
    {
        int optval = on ? 1 : 0;
        if (::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval))) != 0)
        {
            LOG_ERROR("setsockopt SO_REUSEADDR socket:{} failed", _sockfd);
        }
    }

    /**
     * @brief 设置端口复用
     * @param on 是否复用
     */
    void Socket::setReusePort(bool on)
    {
        int optval = on ? 1 : 0;
        if (::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval))) != 0)
        {
            LOG_ERROR("setsockopt SO_REUSEPORT socket:{} failed", _sockfd);
        }
    }

    /**
     * @brief 设置长连接
     * @param on 是否长连接
     */
    void Socket::setKeepAlive(bool on)
    {
        int optval = on ? 1 : 0;
        if (::setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval))) != 0)
        {
            LOG_ERROR("setsockopt SO_KEEPALIVE socket:{} failed", _sockfd);
        }
    }
} // namespace schwi