#pragma once

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

#include "net/poller/Poller.hpp"
#include "base/Timestamp.hpp"

namespace schwi
{
    class EpollPoller : public Poller
    {
    public:
        using EventList = std::vector<struct epoll_event>;
        EpollPoller(EventLoop *loop);
        ~EpollPoller() override;

        Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
        void updateChannel(Channel *channel) override;
        void removeChannel(Channel *channel) override;

    private:
        static const int kInitEventListSize = 16;

        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

        void update(int operation, Channel *channel);

        int _epollfd;
        EventList _events;
    };
} // namespace schwi