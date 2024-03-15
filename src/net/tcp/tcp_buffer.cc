#include "src/net/tcp/tcp_buffer.h"
#include "src/common/log.h"

#include <assert.h>
#include <cstring>
#include <string.h>

namespace mrpc
{

TcpBuffer::TcpBuffer(int size) : m_size(size)
{
    m_buffer.resize(size);
}

TcpBuffer::TcpBuffer(const std::string &buf)
{
    m_buffer.resize(buf.size());
    wirteToBuffer(buf);
}


TcpBuffer::~TcpBuffer()
{
}


int TcpBuffer::readAble()
{
    return m_write_index - m_read_index;
}

int TcpBuffer::writAble()
{
    return int(m_buffer.size()) - m_write_index;
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
    int write_size = int(std::string(buf).length()) < size ? int(std::string(buf).length()) : size;
    if (size == 72) {
        LOG_INFO << "write_size: " << write_size;
        LOG_INFO << "buf: " << buf;
    }

    if (write_size >= writAble()) {
        // 扩容 m_buffer
        int new_size = (int) (1.5 * (m_write_index + write_size));
        resizeBuffer(new_size);
    }
    // memcpy(&m_buffer[m_write_index], buf, write_size);
    std::copy(buf, buf + write_size, m_buffer.begin() + m_write_index);
    moveWriteIndex(write_size);
}

void TcpBuffer::wirteToBuffer(const std::string &buf)
{
    // wirteToBuffer(buf.c_str(), buf.length());

    int write_size = static_cast<int>(buf.length());
    if (write_size >= writAble()) {
        // 扩容 m_buffer
        int new_size = (int) (1.5 * (m_write_index + write_size));
        resizeBuffer(new_size);
    }
    // memcpy(&m_buffer[m_write_index], buf, write_size);
    std::copy(buf.begin(), buf.begin() + write_size, m_buffer.begin() + m_write_index);
    moveWriteIndex(write_size);
}


void TcpBuffer::readFromBuffer(std::vector<char> &re, int size)
{
    if (readAble() == 0) return;

    // 如果 可读的字节数 > 需要读的, 就读size, 反之读readAble()
    int read_size = readAble() > size ? size : readAble();

    std::vector<char> tmp(read_size);
    re.resize(read_size);
    // memcpy(&tmp[0], &m_buffer[m_read_index], read_size);
    std::copy(beginRead(), beginRead() + read_size, tmp.begin());

    re.swap(tmp);
    moveReadIndex(read_size);
}

std::string TcpBuffer::readAsString(int size)
{
    if (size <= 0) return std::string();
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
    if (readAble() == 0) return std::string();

    // 如果 可读的字节数 > 需要读的, 就读size, 反之读readAble()
    int read_size = readAble() > size ? size : readAble();

    std::string tmp;
    tmp.assign(&m_buffer[m_read_index], read_size);

    return tmp;
}

std::string TcpBuffer::peekAsString(int start_index, int end_index)
{
    if (readAble() == 0) return std::string();
    int size = end_index - start_index + 1;

    // 如果 可读的字节数 > 需要读的, 就读size, 反之读readAble()
    int read_size = readAble() > size ? size : readAble();

    std::string tmp;
    tmp.assign(&m_buffer[m_read_index + start_index], read_size);

    return tmp;
}

void TcpBuffer::peekAsVector(std::vector<char> &re, int size)
{
    if (readAble() == 0) return;

    // 如果 可读的字节数 > 需要读的, 就读size, 反之读readAble()
    int read_size = readAble() > size ? size : readAble();
    re.clear();
    re.resize(read_size);
    std::copy(beginRead(), beginRead() + read_size, re.begin());
}

void TcpBuffer::resizeBuffer(int new_size)
{
    std::vector<char> tmp(new_size);
    int count = std::min(new_size, readAble());

    // 将 m_buffer 可读字节copy到tmp
    // memcpy(&tmp[0], &m_buffer[m_read_index], count);
    std::copy(beginRead(), beginRead() + count, tmp.begin());
    m_buffer.swap(tmp);

    m_read_index = 0;
    m_write_index = m_read_index + count;
}

void TcpBuffer::adjustBuffer()
{
    if (m_read_index < int(m_buffer.size() / 3)) return;
    std::vector<char> buffer(m_buffer.size());
    int count = readAble();
    // memcpy(&buffer[0], &m_buffer[m_read_index], count);
    std::copy(beginRead(), beginRead() + count, buffer.begin());

    m_buffer.swap(buffer);
    m_read_index = 0;
    m_write_index = m_read_index + count;

    buffer.clear();
}

void TcpBuffer::moveReadIndex(int size)
{
    int j = m_read_index + size;
    if (j >= int(m_buffer.size())) {
        LOG_ERROR << "moveReadIndex error, invalid size";
        return;
    }

    m_read_index = j;
    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size)
{
    int j = m_write_index + size;
    if (j >= int(m_buffer.size())) {
        LOG_ERROR << "moveWriteIndex error, invalid size";
        return;
    }

    m_write_index = j;
    adjustBuffer();
}

}// namespace mrpc