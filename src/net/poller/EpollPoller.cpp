#include "net/poller/EpollPoller.hpp"
#include "base/base.hpp"

namespace schwi
{
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;

    EpollPoller::EpollPoller(EventLoop *loop)
        : Poller(loop),
          _epollfd(::epoll_create1(EPOLL_CLOEXEC)),
          _events(kInitEventListSize)
    {
        if (_epollfd < 0)
        {
            LOG_FATAL("EpollPoller::EpollPoller()");
        }
    }

    EpollPoller::~EpollPoller()
    {
        ::close(_epollfd);
    }

    Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels)
    {
        int numEvents = ::epoll_wait(_epollfd,
                                     _events.data(),
                                     static_cast<int>(_events.size()),
                                     timeoutMs);
        Timestamp now(Timestamp::now());
        if (numEvents > 0)
        {
            LOG_INFO("EpollPoller::poll() numEvents = {}", numEvents);
            fillActiveChannels(numEvents, activeChannels);
            if (static_cast<size_t>(numEvents) == _events.size())
            {
                _events.resize(_events.size() * 2);
            }
        }
        else if (numEvents == 0)
        {
            LOG_DEBUG("timeout!");
        }
        else
        {
            LOG_ERROR("EpollPoller::poll()");
        }
        return now;
    }

    void EpollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
    {
        for (int i = 0; i < numEvents; ++i)
        {
            Channel *channel = static_cast<Channel *>(_events[i].data.ptr);
            if (channel == nullptr)
            {
                LOG_ERROR("EpollPoller::fillActiveChannels() channel is nullptr");
                continue;
            }
            LOG_DEBUG("EpollPoller::fillActiveChannels() fd = {} events={}", channel->fd(), _events[i].events);
            channel->set_revents(_events[i].events);
            activeChannels->push_back(channel);
        }
    }

    void EpollPoller::updateChannel(Channel *channel)
    {
        const int index = channel->index();
        if (index == kNew || index == kDeleted)
        {
            if (index == kNew)
            {
                int fd = channel->fd();
                _channels[fd] = channel;
            }
            channel->set_index(kAdded);
            update(EPOLL_CTL_ADD, channel);
        }
        else
        {
            if (channel->isNoneEvent())
            {
                update(EPOLL_CTL_DEL, channel);
                channel->set_index(kDeleted);
            }
            else
            {
                update(EPOLL_CTL_MOD, channel);
            }
        }
    }

    void EpollPoller::removeChannel(Channel *channel)
    {
        int fd = channel->fd();
        _channels.erase(fd);
        if (channel->index() == kAdded)
        {
            update(EPOLL_CTL_DEL, channel);
        }
        channel->set_index(kNew);
    }

    void EpollPoller::update(int operation, Channel *channel)
    {
        struct epoll_event event;
        bzero(&event, sizeof event);

        int fd = channel->fd();
        event.events = channel->events();
        event.data.fd = fd;
        event.data.ptr = channel;

        if (::epoll_ctl(_epollfd, operation, fd, &event) < 0)
        {
            if (operation == EPOLL_CTL_DEL)
            {
                LOG_ERROR("epoll_ctl op = {} fd = {}", operation, fd);
            }
            else
            {
                LOG_FATAL("epoll_ctl op = {} fd = {}", operation, fd);
            }
        }
    }
}