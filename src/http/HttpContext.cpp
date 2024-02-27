#include "http/HttpContext.hpp"
#include "net/Buffer.hpp"

namespace schwi
{
    bool HttpContext::processRequestLine(const char *begin, const char *end)
    {
        bool succeed = false;
        const char *start = begin;
        const char *space = std::find(start, end, ' ');

        HttpRequest::Method method = HttpRequest::kInvalid;
        if (space != end && _request.setMethod(start, space))
        {
            start = space + 1;
            space = std::find(start, end, ' ');
            if (space != end)
            {
                const char *question = std::find(start, space, '?');
                if (question != space)
                {
                    _request.setPath(start, question);
                    _request.setQuery(question, space);
                }
                else
                {
                    _request.setPath(start, space);
                }
                start = space + 1;
                succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
                if (succeed)
                {
                    if (*(end - 1) == '1')
                    {
                        _request.setVersion(HttpRequest::kHttp11);
                    }
                    else if (*(end - 1) == '0')
                    {
                        _request.setVersion(HttpRequest::kHttp10);
                    }
                    else
                    {
                        succeed = false;
                    }
                }
            }
        }
        return succeed;
    }

    bool HttpContext::parseRequest(Buffer *buf, Timestamp receiveTime)
    {
        bool ok = true;
        bool hasMore = true;
        while (hasMore)
        {
            if (_state == kExpectRequestLine)
            {
                const char *crlf = buf->findCRLF();
                if (crlf)
                {
                    ok = processRequestLine(buf->peek(), crlf);
                    if (ok)
                    {
                        _request.setReceiveTime(receiveTime);
                        buf->retrieveUntil(crlf + 2);
                        _state = kExpectHeaders;
                    }
                    else
                    {
                        hasMore = false;
                    }
                }
                else
                {
                    hasMore = false;
                }
            }
            else if (_state == kExpectHeaders)
            {
                const char *crlf = buf->findCRLF();
                if (crlf)
                {
                    const char *colon = std::find(buf->peek(), crlf, ':');
                    if (colon != crlf)
                    {
                        _request.addHeader(buf->peek(), colon, crlf);
                    }
                    else
                    {
                        _state = kGotAll;
                        hasMore = false;
                    }
                    buf->retrieveUntil(crlf + 2);
                }
                else
                {
                    hasMore = false;
                }
            }
            else if (_state == kExpectBody)
            {
            }
        }
        return ok;
    }
} // namespace schwi
