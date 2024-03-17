#ifndef MRPC_NET_TCP_TCP_BUFFER_H
#define MRPC_NET_TCP_TCP_BUFFER_H

#include <cstring>
#include <memory>
#include <vector>

#include "src/common/util.h"

namespace mrpc
{

class TcpBuffer
{

public:
    typedef std::shared_ptr<TcpBuffer> s_ptr;
    TcpBuffer(int size);
    TcpBuffer(const std::string &buf);
    ~TcpBuffer();

    /// @brief 获取可读字节数
    /// @return m_write_index - m_read_index
    int readAble();

    /// @brief 获取可写字节数
    /// @return m_size - m_write_index
    int writAble();

    /// @brief 当前 m_read_index
    /// @return m_read_index
    int readIndex();

    /// @brief 当前 m_write_index
    /// @return m_write_index
    int writeIndex();

    /// @brief 获取开始读的it
    /// @return
    auto beginRead() { return m_buffer.begin() + m_read_index; }

    /// @brief 写入m_buffer
    /// @param buf 原始数据
    /// @param size 写入长度
    void wirteToBuffer(const char *buf, int size);

    /// @brief 写入m_buffer
    /// @param buf
    /// @param size
    void wirteToBuffer(const std::string &buf);

    /// @brief 从 m_buffer 中读 size 长度的数据到 re
    /// @param re 写入的数组
    /// @param size 长度
    void readFromBuffer(std::vector<char> &re, int size);

    /// @brief 读取 size 长度到string
    /// @param size
    /// @return string
    std::string readAsString(int size);

    /// @brief 读取全部到string
    /// @return string
    std::string readAllAsString();

    /// @brief 窥视 size 长度的数据, 但是不移动 m_read_index
    /// @param size
    /// @return string
    std::string peekAsString(int size);

    std::string peekAsString(int start_index, int end_index);

    /// @brief 窥视 size 长度的数据, 但是不移动 m_read_index
    /// @param re: vector
    void peekAsVector(std::vector<char> &re, int size);

    /// @brief 扩容 m_buffer
    /// @param new_size 新的大小
    void resizeBuffer(int new_size);

    /// @brief 重新设置 m_read_index
    void adjustBuffer();

    /// @brief 获取 m_buffer size
    size_t getBufferSize() const;

    /// @brief 写入一个int
    /// @tparam T int
    /// @param value 值
    template<typename T>
    void writeInt(T value);

    /// @brief 读取一个int
    /// @tparam T int
    /// @return value
    template<typename T>
    T readInt();

    /// @brief 获取 m_buffer
    /// @return m_buffer
    std::vector<char> buffer() const { return m_buffer; }


public:
    void moveReadIndex(int size);
    void moveWriteIndex(int size);

private:
    int m_read_index { 0 };
    int m_write_index { 0 };
    int m_size { 0 };

    std::vector<char> m_buffer;
};

template<typename T>
void TcpBuffer::writeInt(T value)
{
    static_assert(std::is_integral<T>::value, "T must be an integral type");

    size_t bytesToWrite = sizeof(T);
    if (writAble() < static_cast<int>(bytesToWrite)) {
        // 处理缓冲区中没有足够空间的情况
        int new_size = (writAble() + bytesToWrite) * 2;
        resizeBuffer(new_size);
    }

    // 将 value 转换为网络字节序后再写入到 m_buffer 中
    T netValue = hostToNetwork(value);
    std::memcpy(&m_buffer[m_write_index], &netValue, bytesToWrite);
    moveWriteIndex(bytesToWrite);
}


template<typename T>
T TcpBuffer::readInt()
{
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    size_t bytesToRead = sizeof(T);
    if (readAble() < static_cast<int>(bytesToRead)) {
        // 处理缓冲区中没有足够字节的情况
        throw std::runtime_error("Not enough bytes in buffer");
    }

    T result;
    std::memcpy(&result, &m_buffer[m_read_index], bytesToRead);
    moveReadIndex(bytesToRead);
    return hostToNetwork(result);
}

}// namespace mrpc

#endif//MRPC_NET_TCP_TCP_BUFFER_H