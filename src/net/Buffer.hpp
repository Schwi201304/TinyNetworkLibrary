#pragma once

#include <vector>
#include <string>
#include <algorithm>

namespace schwi
{
    class Buffer
    {
    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize = kInitialSize)
            : _buffer(kCheapPrepend + initialSize),
              _readerIndex(kCheapPrepend),
              _writerIndex(kCheapPrepend)
        {
        }

        size_t readableBytes() const { return _writerIndex - _readerIndex; }
        size_t writableBytes() const { return _buffer.size() - _writerIndex; }
        size_t prependableBytes() const { return _readerIndex; }

        const char *peek() const { return begin() + _readerIndex; }
        void retrieveUntil(const char *end) { retrieve(end - peek()); }
        void retrieve(size_t len)
        {
            if (len < readableBytes())
            {
                _readerIndex += len;
            }
            else
            {
                retrieveAll();
            }
        }

        void retrieveAll()
        {
            _readerIndex = kCheapPrepend;
            _writerIndex = kCheapPrepend;
        }

        std::string retrieveAsString(size_t len)
        {
            std::string result(peek(), len);
            retrieve(len);
            return result;
        }

        std::string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        std::string GetBufferAllAsString()
        {
            size_t len = readableBytes();
            std::string str(peek(), len);
            return str;
        }

        void ensureWritableBytes(size_t len)
        {
            if (writableBytes() < len)
            {
                makeSpace(len);
            }
        }

        void append(const std::string &str)
        {
            append(str.data(), str.size());
        }

        void append(const char *data, size_t len)
        {
            ensureWritableBytes(len);
            std::copy(data, data + len, beginWrite());
            _writerIndex += len;
        }

        const char *findCRLF() const
        {
            const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
            return crlf == beginWrite() ? nullptr : crlf;
        }

        char *beginWrite() { return begin() + _writerIndex; }
        const char *beginWrite() const { return begin() + _writerIndex; }

        ssize_t readFd(int fd, int *savedErrno);
        ssize_t writeFd(int fd, int *savedErrno);

    private:
        char *begin() { return &*(_buffer.begin()); }
        const char *begin() const { return &*(_buffer.begin()); }

        void makeSpace(size_t len);

        std::vector<char> _buffer;
        size_t _readerIndex;
        size_t _writerIndex;
        static const char kCRLF[];
    };
} // namespace schwi