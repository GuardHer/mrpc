#ifndef MRPC_COMMON_LOGSTREAM_H
#define MRPC_COMMON_LOGSTREAM_H

#include <sstream>

namespace mrpc
{

class LogStream
{
public:
    LogStream() = default;
    ~LogStream() = default;

    /// @brief 将日志流转换成 std::string
    /// @return stream_.str()
    std::string toString() const { return stream_.str(); };

    /// @brief 获取当前stream_大小
    /// @return stream_.str().size()
    size_t size() const { return stream_.str().size(); }

public:
    /// @brief 重载 << 运算符, 将数据写入stream_
    /// @tparam T 类型
    /// @param value 值
    template<typename T>
    LogStream &operator<<(T value)
    {
        stream_ << value;
        return *this;
    }

private:
    std::ostringstream stream_;
};

}// namespace mrpc


#endif//MRPC_COMMON_LOGSTREAM_H