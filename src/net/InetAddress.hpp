#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

namespace schwi
{
    /**
     * @brief IP地址和端口
     */
    class InetAddress
    {
    public:
        explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
        InetAddress(const std::string &ip, uint16_t port);
        InetAddress(const struct sockaddr_in &addr) : _addr(addr) {}

        std::string toIp() const;
        std::string toIpPort() const;
        uint16_t toPort() const;

        /**
         * @brief 获取地址
         * @return const struct sockaddr_in&
         */
        const struct sockaddr_in *getSockAddr() const { return &_addr; }
        /**
         * @brief 设置地址
         * @param addr 地址
         * @return void
         */
        void setSockAddr(const struct sockaddr_in &addr) { _addr = addr; }

    private:
        struct sockaddr_in _addr; // 网络字节序
    };
} // namespace schwi