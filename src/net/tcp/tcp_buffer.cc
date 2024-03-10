#include "src/net/tcp/tcp_buffer.h"
#include "src/common/log.h"

#include <string.h>

namespace mrpc
{

TcpBuffer::TcpBuffer(int size) : m_size(size)
{
    m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer()
{
}


int TcpBuffer::readAble()
{
    return m_write_index - m_read_index;
}

int TcpBuffer::wirteAble()
{
    return m_size - m_write_index;
}

int TcpBuffer::readIndex()
{
    return m_read_index;
}

int TcpBuffer::writeIndex()
{
    return m_write_index;
}

size_t TcpBuffer::getBufferSize() const
{
    return m_buffer.size();
}

void TcpBuffer::wirteToBuffer(const char *buf, int size)
{
    // 检查 sizeof buf < size, 如果满足, 就写入全部buf, 反之写入size
    int write_size = sizeof(buf) < size ? sizeof(buf) : size;

    if (write_size > wirteAble()) {
        // 扩容 m_buffer
        int new_size = (int) (1.5 * (m_write_index + write_size));
        resizeBuffer(new_size);
    }
    memcpy(&m_buffer[m_write_index], buf, write_size);
    moveWriteIndex(write_size);
}

void TcpBuffer::readFromBuffer(std::vector<char> &re, int size)
{
    if (readAble() == 0) return;

    // 如果 可读的字节数 > 需要读的, 就读size, 反之读readAble()
    int read_size = readAble() > size ? size : readAble();

    std::vector<char> tmp(read_size);
    memcpy(&tmp[0], &m_buffer[m_read_index], read_size);

    re.swap(tmp);
    moveReadIndex(read_size);
}

std::string TcpBuffer::readAsString(int size)
{
    std::vector<char> tmp;
    readFromBuffer(tmp, size);

    std::string res(tmp.begin(), tmp.end());

    return res;
}

std::string TcpBuffer::readAllAsString()
{
    return readAsString(readAble());
}

std::string TcpBuffer::peekAsString(int size)
{
    if (readAble() == 0) return;

    // 如果 可读的字节数 > 需要读的, 就读size, 反之读readAble()
    int read_size = readAble() > size ? size : readAble();

    std::string tmp;
    memcpy(&tmp[0], &m_buffer[m_read_index], read_size);

    return tmp;
}

void TcpBuffer::resizeBuffer(int new_size)
{
    std::vector<char> tmp(new_size);
    int count = std::min(new_size, readAble());

    // 将 m_buffer 可读字节copy到tmp
    memcpy(&tmp[0], &m_buffer[m_read_index], count);
    m_buffer.swap(tmp);

    m_read_index = 0;
    m_write_index = m_read_index + count;
}

void TcpBuffer::adjustBuffer()
{
    if (m_read_index < int(m_buffer.size() / 3)) return;
    std::vector<char> buffer(m_buffer.size());
    int count = readAble();
    memcpy(&buffer[0], &m_buffer[m_read_index], count);
    m_buffer.swap(buffer);
    m_read_index = 0;
    m_write_index = m_read_index + count;

    buffer.clear();
}

void TcpBuffer::moveReadIndex(int size)
{
    int j = m_read_index + size;
    if (j >= m_buffer.size()) {
        LOG_ERROR << "moveReadIndex error, invalid size";
        return;
    }

    m_read_index = j;
    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size)
{
    int j = m_write_index + size;
    if (j >= m_buffer.size()) {
        LOG_ERROR << "moveWriteIndex error, invalid size";
        return;
    }

    m_write_index = j;
    adjustBuffer();
}

}// namespace mrpc