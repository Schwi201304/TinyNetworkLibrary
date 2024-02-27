#include "net/EventLoop.hpp"
#include "net/TcpServer.hpp"
#include "log/Logger.hpp"
#include "log/LogStream.hpp"
#include "log/AsyncLogFile.hpp"
#include "base/base.hpp"

using namespace schwi;
using namespace std;

void InitGlobalLogger()
{
    auto logConsole = std::make_shared<LogConsole>();
    auto logger = std::make_shared<Logger>(Logger::DEBUG, logConsole);
    GlobalLogger::Instance().setLogger(logger);
}

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &listenAddr)
        : server_(loop, listenAddr, "EchoServer", TcpServer::kReusePort)
    {
        LOG_INFO("EchoServer is created : {}", listenAddr.toIpPort());
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void start()
    {
        LOG_INFO("EchoServer is starting");
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("EchoServer - {} -> {} is {}",
                     conn->peerAddress().toIpPort(),
                     conn->localAddress().toIpPort(),
                     "UP");
        }
        else
        {
            LOG_INFO("EchoServer - {} -> {} is {}",
                     conn->peerAddress().toIpPort(),
                     conn->localAddress().toIpPort(),
                     "DOWN");
        }
    }

    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                   Timestamp receiveTime)
    {
        string msg(buf->retrieveAllAsString());
        LOG_INFO("EchoServer - received {} bytes from connection [{}]",
                 msg.size(), conn->name());
        conn->send(msg);
    }

    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    InitGlobalLogger();
    LOG_INFO("pid = {}", getpid());
    EventLoop loop;
    InetAddress listenAddr(8080);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();

    return 0;
}