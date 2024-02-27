#include "net/Channel.hpp"
#include "net/EventLoop.hpp"
#include "base/base.hpp"

namespace schwi
{
    const int Channel::kNoneEvent = 0;
    const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
    const int Channel::kWriteEvent = EPOLLOUT;

    Channel::Channel(EventLoop *loop, int fd)
        : _loop(loop),
          _fd(fd),
          _events(0),
          _revents(0),
          _index(-1),
          _tied(false)
    {
    }

    Channel::~Channel()
    {
    }

    void Channel::handleEvent(Timestamp receiveTime)
    {
        if (_tied)
        {
            std::shared_ptr<void> guard = _tie.lock();
            if (guard)
            {
                handleEventWithGuard(receiveTime);
            }
        }
        else
        {
            handleEventWithGuard(receiveTime);
        }
    }

    void Channel::tie(const std::shared_ptr<void> &obj)
    {
        _tie = obj;
        _tied = true;
    }

    void Channel::enableReading()
    {
        _events |= kReadEvent;
        update();
    }

    void Channel::disableReading()
    {
        _events &= ~kReadEvent;
        update();
    }

    void Channel::enableWriting()
    {
        _events |= kWriteEvent;
        update();
    }

    void Channel::disableWriting()
    {
        _events &= ~kWriteEvent;
        update();
    }

    void Channel::disableAll()
    {
        _events &= kNoneEvent;
        update();
    }

    void Channel::update()
    {
        _loop->updateChannel(this);
    }

    void Channel::remove()
    {
        _loop->removeChannel(this);
    }

    void Channel::handleEventWithGuard(Timestamp receiveTime)
    {
        // 写端关闭
        if ((_revents & EPOLLHUP) && !(_revents & EPOLLIN))
        {
            if (_closeCallback)
                _closeCallback();
        }

        // 错误
        if (_revents & EPOLLERR)
        {
            LOG_ERROR("Channel::handleEventWithGuard() EPOLLERR: fd = {}", _fd);
            if (_errorCallback)
                _errorCallback();
        }

        // 读事件
        if (_revents & (EPOLLIN | EPOLLPRI))
        {
            if (_readCallback)
                _readCallback(receiveTime);
        }

        // 写事件
        if (_revents & EPOLLOUT)
        {
            if (_writeCallback)
                _writeCallback();
        }
    }
} // namespace schwi