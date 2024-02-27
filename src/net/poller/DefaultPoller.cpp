#include "net/poller/Poller.hpp"
#include "net/poller/EpollPoller.hpp"

#include <stdlib.h>

namespace schwi
{
    Poller *Poller::newDefaultPoller(EventLoop *loop)
    {
        return new EpollPoller(loop);
    }
} // namespace schwi