#include "http/HttpServer.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpContext.hpp"
#include "base/base.hpp"

#include <memory>

namespace schwi
{
    void defaultHttpCallback(const HttpRequest &, HttpResponse *resp)
    {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }

    HttpServer::HttpServer(EventLoop *loop,
                           const InetAddress &listenAddr,
                           const std::string &name,
                           TcpServer::Option option)
        : _server(loop, listenAddr, name, option),
          _httpCallback(defaultHttpCallback)
    {
        _server.setConnectionCallback(
            std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
        _server.setMessageCallback(
            std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        _server.setThreadNum(4);
    }

    HttpServer::~HttpServer()
    {
    }

    void HttpServer::start()
    {
        LOG_TRACE("HttpServer[{}] starts", _server.ipPort());
        _server.start();
    }

    void HttpServer::onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("HttpServer - {} -> {} is {}",
                     conn->peerAddress().toIpPort(),
                     conn->localAddress().toIpPort(),
                     (conn->connected() ? "UP" : "DOWN"));
        }
    }

    void HttpServer::onMessage(const TcpConnectionPtr &conn,
                               Buffer *buf,
                               Timestamp receiveTime)
    {
        std::unique_ptr<HttpContext> context(new HttpContext());
        if (!context->parseRequest(buf, receiveTime))
        {
            LOG_INFO("HttpServer - bad request from {}", conn->peerAddress().toIpPort());
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
        }

        if (context->gotAll())
        {
            LOG_DEBUG("HttpServer - request: {} {}",
                      context->request().methodString(),
                      context->request().path());
            onRequest(conn, context->request());
            context.reset();
        }
    }

    void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req)
    {
        const std::string &connection = req.getHeader("Connection");
        bool close = connection == "close" ||
                     (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
        LOG_DEBUG("HttpServer - request: {} {} {}",
                  req.methodString(),
                  req.path(),
                  close ? "close" : "keep-alive");
        HttpResponse response(close);
        _httpCallback(req, &response);
        Buffer buf;
        response.appendToBuffer(&buf);
        conn->send(&buf);
        if (response.closeConnection())
        {
            conn->shutdown();
        }
    }
} // namespace schwi
