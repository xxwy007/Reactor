#include "Buffer.h"

Buffer::Buffer(size_t initialSize)
    : m_buffer(initialSize), m_readIndex(0), m_writeIndex(0)
{
}

size_t Buffer::readableBytes() const
{
    return m_writeIndex - m_readIndex;
}

size_t Buffer::writableBytes() const
{
    return m_buffer.size() - m_writeIndex;
}

const char *Buffer::peek() const
{
    return m_buffer.data() + m_readIndex;
}

void Buffer::retrieve(size_t len)
{
    if (len < readableBytes())
    {
        m_readIndex += len;
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveAll()
{
    m_readIndex = 0;
    m_writeIndex = 0;
}

void Buffer::append(const char *data, size_t len)
{
    ensureWritableBytes(len);

    std::copy(data, data + len, m_buffer.begin() + m_writeIndex);

    m_writeIndex += len;
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len)
    {
        m_buffer.resize(m_writeIndex + len);
    }
}