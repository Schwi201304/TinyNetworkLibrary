#include "base/CurrentThread.hpp"

namespace schwi
{
    namespace CurrentThread
    {
        thread_local int t_cachedTid = 0; 
    } // namespace CurrentThread
} // namespace schwi