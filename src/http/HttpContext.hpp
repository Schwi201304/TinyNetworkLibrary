#pragma once

#include "http/HttpRequest.hpp"

namespace schwi
{
    class Buffer;

    class HttpContext
    {
    public:
        enum HttpRequestParseState
        {
            kExpectRequestLine,
            kExpectHeaders,
            kExpectBody,
            kGotAll
        };

        HttpContext()
            : _state(kExpectRequestLine)
        {
        }

        bool parseRequest(Buffer *buf, Timestamp receiveTime);

        bool gotAll() const { return _state == kGotAll; }

        void reset()
        {
            _state = kExpectRequestLine;
            HttpRequest dummy;
            _request.swap(dummy);
        }

        const HttpRequest &request() const { return _request; }

        HttpRequest &request() { return _request; }

    private:
        bool processRequestLine(const char *begin, const char *end);

        HttpRequestParseState _state;
        HttpRequest _request;
    };
} // namespace schwi
