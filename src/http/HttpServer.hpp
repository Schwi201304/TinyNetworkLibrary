#pragma once

#include "net/TcpServer.hpp"
#include "base/noncopyable.hpp"

#include <string>

namespace schwi
{
    class HttpRequest;
    class HttpResponse;

    class HttpServer : noncopyable
    {
    public:
        using HttpCallback = std::function<void(const HttpRequest &, HttpResponse *)>;

        HttpServer(EventLoop *loop,
                   const InetAddress &listenAddr,
                   const std::string &name,
                   TcpServer::Option option = TcpServer::kNoReusePort);

        ~HttpServer();

        EventLoop *getLoop() const { return _server.getLoop(); }

        void setHttpCallback(const HttpCallback &cb)
        {
            _httpCallback = cb;
        }

        void start();

    private:
        void onConnection(const TcpConnectionPtr &conn);
        void onMessage(const TcpConnectionPtr &conn,
                       Buffer *buf,
                       Timestamp receiveTime);
        void onRequest(const TcpConnectionPtr &conn, const HttpRequest &req);

        TcpServer _server;
        HttpCallback _httpCallback;
    };
} // namespace schwi