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
    std::string toString() const { return stream_.str(); };
    size_t size() const { return stream_.str().size(); }

public:
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