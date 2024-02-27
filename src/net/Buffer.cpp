#include "net/Buffer.hpp"
#include "base/base.hpp"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

namespace schwi
{
    const char Buffer::kCRLF[] = "\r\n";

    ssize_t Buffer::readFd(int fd, int *savedErrno)
    {
        char extrabuf[65536] = {0};

        struct iovec vec[2];
        const size_t writable = writableBytes();

        vec[0].iov_base = begin() + _writerIndex;
        vec[0].iov_len = writable;
        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof(extrabuf);

        const ssize_t n = ::readv(fd, vec, 2);
        if (n < 0)
        {
            *savedErrno = errno;
        }
        else if (static_cast<size_t>(n) <= writable)
        {
            _writerIndex += n;
        }
        else
        {
            _writerIndex = _buffer.size();
            append(extrabuf, n - writable);
        }
        return n;
    }

    ssize_t Buffer::writeFd(int fd, int *savedErrno)
    {
        ssize_t n = ::write(fd, peek(), readableBytes());
        if (n < 0)
        {
            *savedErrno = errno;
        }
        return n;
    }

    void Buffer::makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            _buffer.resize(_writerIndex + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + _readerIndex, begin() + _writerIndex, begin() + kCheapPrepend);
            _readerIndex = kCheapPrepend;
            _writerIndex = _readerIndex + readable;
        }
    }
} // namespace schwi