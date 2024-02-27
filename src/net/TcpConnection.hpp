#pragma once

#include <memory>
#include <string>
#include <atomic>

#include "base/noncopyable.hpp"
#include "base/Timestamp.hpp"
#include "net/Buffer.hpp"
#include "net/Callback.hpp"
#include "net/InetAddress.hpp"

namespace schwi
{
    class Channel;
    class EventLoop;
    class Socket;

    class TcpConnection : noncopyable,
                          public std::enable_shared_from_this<TcpConnection>
    {
    public:
        TcpConnection(EventLoop *loop,
                      const std::string &name,
                      int sockfd,
                      const InetAddress &localAddr,
                      const InetAddress &peerAddr);
        ~TcpConnection();

        EventLoop *getLoop() const { return _loop; }
        const std::string &name() const { return _name; }
        const InetAddress &localAddress() const { return _localAddr; }
        const InetAddress &peerAddress() const { return _peerAddr; }
        bool connected() const { return _state == kConnected; }

        void send(const std::string &message);
        void send(Buffer *message);

        void shutdown();

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            _connectionCallback = cb;
        }
        void setMessageCallback(const MessageCallback &cb)
        {
            _messageCallback = cb;
        }
        void setWriteCompleteCallback(const WriteCompleteCallback &cb)
        {
            _writeCompleteCallback = cb;
        }
        void setCloseCallback(const CloseCallback &cb)
        {
            _closeCallback = cb;
        }
        void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
        {
            _highWaterMarkCallback = cb;
            _highWaterMark = highWaterMark;
        }

        void connectEstablished();
        void connectDestroyed();

    private:
        enum StateE
        {
            kDisconnected,
            kConnecting,
            kConnected,
            kDisconnecting
        };

        void setState(StateE s) { _state = s; }
        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();

        void sendInLoop(const std::string &message);
        void sendInLoop(const void *message, size_t len);
        void shutdownInLoop();

        EventLoop *_loop;
        std::string _name;
        std::atomic_int _state;
        bool _reading;

        std::unique_ptr<Socket> _socket;
        std::unique_ptr<Channel> _channel;

        const InetAddress _localAddr;
        const InetAddress _peerAddr;

        ConnectionCallback _connectionCallback;
        MessageCallback _messageCallback;
        WriteCompleteCallback _writeCompleteCallback;
        CloseCallback _closeCallback;
        HighWaterMarkCallback _highWaterMarkCallback;
        size_t _highWaterMark;

        Buffer _inputBuffer;
        Buffer _outputBuffer;
    };

} // namespace schwi
