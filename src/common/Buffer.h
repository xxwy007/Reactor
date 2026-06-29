#ifndef BUFFER_H
#define BUFFER_H
#include <vector>

//参考muduo
class Buffer
{
public:
    Buffer(size_t initialSize = 1024);
    size_t readableBytes() const;
    size_t writableBytes() const;
    const char *peek() const;
    void retrieve(size_t len);
    void retrieveAll();
    void append(const char *data, size_t len);
    void ensureWritableBytes(size_t len);
private:
    std::vector<char> m_buffer;

    size_t m_readIndex;
    size_t m_writeIndex;
};

#endif