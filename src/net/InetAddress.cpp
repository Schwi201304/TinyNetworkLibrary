#include "net/InetAddress.hpp"
#include <cstring>

namespace schwi
{
    /**
     * @brief 构造函数
     * @param port 端口
     * @param ip IP地址
     */
    InetAddress::InetAddress(uint16_t port, std::string ip)
    {
        ::bzero(&_addr, sizeof(_addr));
        _addr.sin_family = AF_INET;
        _addr.sin_port = ::htons(port);
        _addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    /**
     * @brief 构造函数
     * @param ip IP地址
     * @param port 端口
     */
    InetAddress::InetAddress(const std::string &ip, uint16_t port)
    {
        ::bzero(&_addr, sizeof(_addr));
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        ::inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr);
    }

    /**
     * @brief 获取IP地址
     * @return std::string
     */
    std::string InetAddress::toIp() const
    {
        char buf[64] = {0};
        ::inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf));
        return buf;
    }

    /**
     * @brief 获取IP地址和端口
     * @return std::string
     */
    std::string InetAddress::toIpPort() const
    {
        char buf[64] = {0};
        ::inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf));
        uint16_t port = ntohs(_addr.sin_port);
        return std::string(buf) + ":" + std::to_string(port);
    }

    /**
     * @brief 获取端口
     * @return uint16_t
     */
    uint16_t InetAddress::toPort() const
    {
        return ntohs(_addr.sin_port);
    }
} // namespace schwi