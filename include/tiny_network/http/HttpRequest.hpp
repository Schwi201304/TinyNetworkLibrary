#pragma once

#include "base/noncopyable.hpp"
#include "base/Timestamp.hpp"
#include <unordered_map>
#include <string>

namespace schwi
{
    class HttpRequest
    {
    public:
        enum Method
        {
            kInvalid,
            kGet,
            kPost,
            kHead,
            kPut,
            kDelete
        };

        enum Version
        {
            kUnknown,
            kHttp10,
            kHttp11
        };

        HttpRequest()
            : _method(kInvalid),
              _version(kUnknown)
        {
        }

        void setVersion(Version v)
        {
            _version = v;
        }

        Version getVersion() const
        {
            return _version;
        }

        bool setMethod(const char *start, const char *end)
        {
            std::string m(start, end);
            if (m == "GET")
            {
                _method = kGet;
            }
            else if (m == "POST")
            {
                _method = kPost;
            }
            else if (m == "HEAD")
            {
                _method = kHead;
            }
            else if (m == "PUT")
            {
                _method = kPut;
            }
            else if (m == "DELETE")
            {
                _method = kDelete;
            }
            else
            {
                _method = kInvalid;
            }
            return _method != kInvalid;
        }

        Method method() const
        {
            return _method;
        }

        const char *methodString() const
        {
            const char *result = "UNKNOWN";
            switch (_method)
            {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kPut:
                result = "PUT";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
            }
            return result;
        }

        void setPath(const char *start, const char *end)
        {
            _path.assign(start, end);
        }

        const std::string &path() const
        {
            return _path;
        }

        void setQuery(const char *start, const char *end)
        {
            _query.assign(start, end);
        }

        const std::string &query() const
        {
            return _query;
        }

        void setReceiveTime(Timestamp t)
        {
            _receiveTime = t;
        }

        Timestamp receiveTime() const
        {
            return _receiveTime;
        }

        void addHeader(const char *start, const char *colon, const char *end)
        {
            std::string field(start, colon);
            ++colon;
            while (colon < end && isspace(*colon))
            {
                ++colon;
            }
            std::string value(colon, end);
            while (!value.empty() && isspace(value[value.size() - 1]))
            {
                value.resize(value.size() - 1);
            }
            _headers[field] = value;
        }

        std::string getHeader(const std::string &field) const
        {
            std::string result;
            auto it = _headers.find(field);
            if (it != _headers.end())
            {
                result = it->second;
            }
            return result;
        }

        const std::unordered_map<std::string, std::string> &headers() const
        {
            return _headers;
        }

        void swap(HttpRequest &rhs)
        {
            std::swap(_method, rhs._method);
            std::swap(_version, rhs._version);
            _path.swap(rhs._path);
            _query.swap(rhs._query);
            std::swap(_receiveTime, rhs._receiveTime);
            _headers.swap(rhs._headers);
        }

    private:
        Method _method;
        Version _version;
        std::string _path;
        std::string _query;
        Timestamp _receiveTime;
        std::unordered_map<std::string, std::string> _headers;
    };

} // namespace schwi
