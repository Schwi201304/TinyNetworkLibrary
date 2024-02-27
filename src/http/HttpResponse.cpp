#include "http/HttpResponse.hpp"
#include "net/Buffer.hpp"

#include <stdio.h>
#include <string.h>

namespace schwi
{
    void HttpResponse::appendToBuffer(Buffer *output) const
    {
        char buf[32];
        snprintf(buf, sizeof buf, "HTTP/1.1 %d ", _statusCode);
        output->append(buf);
        output->append(_statusMessage);
        output->append("\r\n");

        if (_closeConnection)
        {
            output->append("Connection: close\r\n");
        }
        else
        {
            snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", _body.size());
            output->append(buf);
            output->append("Connection: Keep-Alive\r\n");
        }

        for (const auto &header : _headers)
        {
            output->append(header.first);
            output->append(": ");
            output->append(header.second);
            output->append("\r\n");
        }

        output->append("\r\n");
        output->append(_body);
    }
} // namespace schwi
