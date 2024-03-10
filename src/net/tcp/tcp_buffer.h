#ifndef MRPC_NET_TCP_TCP_BUFFER_H
#define MRPC_NET_TCP_TCP_BUFFER_H

#include <memory>
#include <vector>

namespace mrpc
{

class TcpBuffer
{

public:
    typedef std::shared_ptr<TcpBuffer> s_ptr;
    TcpBuffer(int size);
    ~TcpBuffer();

    /// @brief 获取可读字节数
    /// @return m_write_index - m_read_index
    int readAble();

    /// @brief 获取可写字节数
    /// @return m_size - m_write_index
    int wirteAble();

    /// @brief 当前 m_read_index
    /// @return m_read_index
    int readIndex();

    /// @brief 当前 m_write_index
    /// @return m_write_index
    int writeIndex();

    /// @brief 写入m_buffer
    /// @param buf 原始数据
    /// @param size 写入长度
    void wirteToBuffer(const char *buf, int size);

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

    /// @brief 扩容 m_buffer
    /// @param new_size 新的大小
    void resizeBuffer(int new_size);

    /// @brief 重新设置 m_read_index
    void adjustBuffer();

    /// @brief 获取 m_buffer size
    size_t getBufferSize() const;

private:
    void moveReadIndex(int size);
    void moveWriteIndex(int size);

private:
    int m_read_index{0};
    int m_write_index{0};
    int m_size{0};

    std::vector<char> m_buffer;
};

}// namespace mrpc

#endif//MRPC_NET_TCP_TCP_BUFFER_H