#pragma once

#include "base/noncopyable.hpp"

namespace schwi
{
    template <typename T>
    class Singleton : noncopyable
    {
    public:
        static T &Instance()
        {
            static T _value;
            return _value;
        }
    }; // class Singleton
} // namespace schwi