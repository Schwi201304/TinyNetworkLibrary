#pragma once

#include "base/noncopyable.hpp"
#include "net/Socket.hpp"
#include "net/Channel.hpp"

namespace schwi
{
    class EventLoop;
    class InetAddress;

    class Acceptor : noncopyable
    {
    public:
        using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;

        Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
        ~Acceptor();

        void setNewConnectionCallback(const NewConnectionCallback &cb)
        {
            _newConnectionCallback = cb;
        }

        bool listenning() const { return _listenning; }
        void listen();

    private:
        void handleRead();

        EventLoop *_loop;
        Socket _acceptSocket;
        Channel _acceptChannel;
        NewConnectionCallback _newConnectionCallback;
        bool _listenning;
    };
} // namespace schwi
