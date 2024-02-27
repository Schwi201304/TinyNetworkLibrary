#pragma once

#include <memory>
#include <functional>
#include <string>
#include <atomic>
#include <unordered_map>

#include "base/noncopyable.hpp"
#include "net/EventLoop.hpp"
#include "net/EventLoopThreadPool.hpp"
#include "net/Acceptor.hpp"
#include "net/InetAddress.hpp"
#include "net/Callback.hpp"
#include "net/TcpConnection.hpp"

namespace schwi
{
    class TcpServer : noncopyable
    {
    public:
        using ThreadInitCallback = std::function<void(EventLoop *)>;

        enum Option
        {
            kNoReusePort,
            kReusePort,
        };

        TcpServer(EventLoop *loop,
                  const InetAddress &listenAddr,
                  const std::string &name,
                  Option option = kNoReusePort);
        ~TcpServer();

        void setThreadInitCallback(const ThreadInitCallback &cb)
        {
            _threadInitCallback = cb;
        }
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

        void start();
        void setThreadNum(int numThreads);
        EventLoop *getLoop() const { return _loop; }
        const std::string &ipPort() const { return _ipPort; }
        const std::string &name() const { return _name; }

    private:
        void newConnection(int sockfd, const InetAddress &peerAddr);
        void removeConnection(const TcpConnectionPtr &conn);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

        using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

        EventLoop *_loop;
        const std::string _ipPort;
        const std::string _name;
        std::unique_ptr<Acceptor> _acceptor;

        std::shared_ptr<EventLoopThreadPool> _threadPool;

        ConnectionCallback _connectionCallback;
        MessageCallback _messageCallback;
        WriteCompleteCallback _writeCompleteCallback;
        ThreadInitCallback _threadInitCallback;

        std::atomic<int> _started;
        int _nextConnId;
        ConnectionMap _connections;
    };
} // namespace schwi
