#include "net/EventLoop.hpp"
#include "net/poller/Poller.hpp"
#include "base/base.hpp"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>

namespace schwi
{
    thread_local EventLoop *t_loopInThisThread = nullptr;

    const int kPollTimeMs = 10000;

    int createEventfd()
    {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0)
        {
            LOG_ERROR("Failed in eventfd");
            abort();
        }
        return evtfd;
    }

    EventLoop::EventLoop()
        : _looping(false),
          _quit(false),
          _callingPendingFunctors(false),
          _threadId(CurrentThread::tid()),
          _poller(Poller::newDefaultPoller(this)),
          _timerQueue(new TimerQueue(this)),
          _wakeupFd(createEventfd()),
          _wakeupChannel(new Channel(this, _wakeupFd)),
          _currentActiveChannel(nullptr)
    {
        LOG_DEBUG("EventLoop created {} the index is {}", this, _threadId);
        LOG_DEBUG("EventLoop created wakeupFd = {}", _wakeupFd);
        if (t_loopInThisThread)
        {
            LOG_FATAL("Another EventLoop {} exists in this thread {}", t_loopInThisThread, _threadId);
        }
        else
        {
            t_loopInThisThread = this;
        }
        _wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
        _wakeupChannel->enableReading();
    }

    EventLoop::~EventLoop()
    {
        LOG_DEBUG("EventLoop {} of thread {} destructs in thread {}", this, _threadId, CurrentThread::tid());
        _wakeupChannel->disableAll();
        _wakeupChannel->remove();
        ::close(_wakeupFd);
        t_loopInThisThread = nullptr;
    }

    void EventLoop::loop()
    {
        _looping = true;
        _quit = false;
        LOG_INFO("EventLoop {} start looping", this);

        while (!_quit)
        {
            _activeChannels.clear();
            _pollReturnTime = _poller->poll(kPollTimeMs, &_activeChannels);
            for (Channel *channel : _activeChannels)
            {
                channel->handleEvent(_pollReturnTime);
            }
            doPendingFunctors();
        }

        LOG_INFO("EventLoop {} stop looping", this);
        _looping = false;
    }

    void EventLoop::quit()
    {
        _quit = true;
        if (isInLoopThread())
        {
            wakeup();
        }
    }

    void EventLoop::runInLoop(Functor cb)
    {
        if (isInLoopThread())
        {
            cb();
        }
        else
        {
            queueInLoop(cb);
        }
    }

    void EventLoop::queueInLoop(Functor cb)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _pendingFunctors.emplace_back(cb);
        }

        if (!isInLoopThread() || _callingPendingFunctors)
        {
            wakeup();
        }
    }

    void EventLoop::wakeup()
    {
        uint64_t one = 1;
        ssize_t n = ::write(_wakeupFd, &one, sizeof one);
        if (n != sizeof one)
        {
            LOG_ERROR("EventLoop::wakeup() writes {} bytes instead of 8", n);
        }
    }

    void EventLoop::updateChannel(Channel *channel)
    {
        _poller->updateChannel(channel);
    }

    void EventLoop::removeChannel(Channel *channel)
    {
        _poller->removeChannel(channel);
    }

    void EventLoop::hasChannel(Channel *channel)
    {
        _poller->hasChannel(channel);
    }

    void EventLoop::runAt(const Timestamp &time, Functor cb)
    {
        _timerQueue->addTimer(std::move(cb), time, 0.0);
    }

    void EventLoop::runAfter(double delay, Functor cb)
    {
        Timestamp time(addTime(Timestamp::now(), delay));
        _timerQueue->addTimer(std::move(cb), time, 0.0);
    }

    void EventLoop::runEvery(double interval, Functor cb)
    {
        Timestamp time(addTime(Timestamp::now(), interval));
        _timerQueue->addTimer(std::move(cb), time, interval);
    }

    void EventLoop::handleRead()
    {
        uint64_t one = 1;
        ssize_t n = ::read(_wakeupFd, &one, sizeof one);
        if (n != sizeof one)
        {
            LOG_ERROR("EventLoop::handleRead() reads {} bytes instead of 8", n);
        }
    }

    void EventLoop::doPendingFunctors()
    {
        std::vector<Functor> functors;
        _callingPendingFunctors = true;

        {
            std::lock_guard<std::mutex> lock(_mutex);
            functors.swap(_pendingFunctors);
        }

        for (const Functor &functor : functors)
        {
            functor();
        }
        _callingPendingFunctors = false;
    }
}
