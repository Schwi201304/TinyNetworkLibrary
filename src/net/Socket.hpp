#pragma once

#include "base/noncopyable.hpp"
#include "net/InetAddress.hpp"

namespace schwi
{
    class Socket : noncopyable
    {
    public:
        explicit Socket(int sockfd) : _sockfd(sockfd) {}
        ~Socket();

        int fd() const { return _sockfd; } // 返回文件描述符

        void bindAddress(const InetAddress &localaddr); // 绑定地址
        void listen();                                  // 监听
        int accept(InetAddress *peeraddr);              // 接受连接

        void shutdownWrite(); // 设置半关闭

        void setTcpNoDelay(bool on); // 设置Nagel算法
        void setReuseAddr(bool on);  // 设置地址复用
        void setReusePort(bool on);  // 设置端口复用
        void setKeepAlive(bool on);  // 设置长连接

    private:
        const int _sockfd;
    }; // class Socket

} // namespace schwi
