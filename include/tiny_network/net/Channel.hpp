#pragma once

#include <functional>
#include <memory>
#include <sys/epoll.h>

#include "base/noncopyable.hpp"

namespace schwi
{
    class EventLoop;
    class Timestamp;

    class Channel : noncopyable
    {
    public:
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(Timestamp)>;

        Channel(EventLoop *loop, int fd);
        ~Channel();

        void handleEvent(Timestamp receiveTime); // 处理事件

        void setReadCallback(const ReadEventCallback &cb) { _readCallback = std::move(cb); } // 设置读回调
        void setWriteCallback(const EventCallback &cb) { _writeCallback = std::move(cb); }   // 设置写回调
        void setCloseCallback(const EventCallback &cb) { _closeCallback = std::move(cb); }   // 设置关闭回调
        void setErrorCallback(const EventCallback &cb) { _errorCallback = std::move(cb); }   // 设置错误回调

        void tie(const std::shared_ptr<void> &obj); // 绑定对象

        int fd() const { return _fd; }                  // 返回封装的fd
        int events() const { return _events; }          // 返回感兴趣的事件
        void set_revents(int revt) { _revents = revt; } // 设置Poller返回的发生事件

        void enableReading();  // 启用读
        void disableReading(); // 禁用读
        void enableWriting();  // 启用写
        void disableWriting(); // 禁用写
        void disableAll();     // 禁用所有

        bool isNoneEvent() const { return _events == kNoneEvent; } // 是否无事件
        bool isWriting() const { return _events & kWriteEvent; }   // 是否写事件
        bool isReading() const { return _events & kReadEvent; }    // 是否读事件

        int index() { return _index; }            // 获取索引
        void set_index(int idx) { _index = idx; } // 设置索引

        EventLoop *ownerLoop() { return _loop; } // 获取事件循环
        void remove();

    private:
        void update();
        void handleEventWithGuard(Timestamp receiveTime);

        static const int kNoneEvent;  // 无事件
        static const int kReadEvent;  // 读事件
        static const int kWriteEvent; // 写事件

        EventLoop *_loop; // 当前Channel属于的EventLoop
        const int _fd;    // fd, Poller监听对象
        int _events;      // 注册fd感兴趣的事件
        int _revents;     // poller返回的具体发生的事件
        int _index;       // 在Poller上注册的情况

        std::weak_ptr<void> _tie; // 弱指针指向TcpConnection
        bool _tied;               // 标志此 Channel 是否被调用过 Channel::tie 方法

        ReadEventCallback _readCallback;
        EventCallback _writeCallback;
        EventCallback _closeCallback;
        EventCallback _errorCallback;
    };
} // namespace schwi