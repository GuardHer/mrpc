#ifndef MRPC_COMMON_UTIL_H
#define MRPC_COMMON_UTIL_H
#include <endian.h>
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <type_traits>
#include <unistd.h>

namespace mrpc
{

/// 获取当前进程id
int32_t getPid();

/// 获取当前线程id
int32_t getThreadId();

/// 获取当前时间 ms
int64_t getNowMs();

/// @brief 获取当前时间的字符串, 精确到 ms
/// @return "%Y-%m-%d %H:%M:%S.xxx"
std::string getCurrentTimeMillisecondsString();

/// @brief 获取当前时间的字符串, 精确到 day
/// @return "%Y%m%d"
std::string getCurrentTimeDayString();

/// 网络字节序转主机字节序
uint32_t networkToHost32(const char *net32);
template<typename T>
T networkToHost(T value)
{
    if constexpr (std::is_integral<T>::value) {
        // 判断整数类型的大小
        if constexpr (sizeof(T) == sizeof(uint32_t)) {
            // 如果 T 的大小是 4 字节，则使用 be32toh 函数
            return static_cast<T>(be32toh(static_cast<uint32_t>(value)));
        } else if constexpr (sizeof(T) == sizeof(uint64_t)) {
            // 如果 T 的大小是 8 字节，则使用 be64toh 函数
            return static_cast<T>(be64toh(static_cast<uint64_t>(value)));
        } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
            // 如果 T 的大小是 2 字节，则使用 be16toh 函数
            return static_cast<T>(be16toh(static_cast<uint16_t>(value)));
        } else {
            // 如果 T 的大小既不是 4 字节也不是 8 字节，则抛出编译时错误
            static_assert(sizeof(T) == sizeof(uint16_t) || sizeof(T) == sizeof(uint32_t) || sizeof(T) == sizeof(uint64_t),
                          "Unsupported integral type size");
        }
    } else {
        // 如果不是整数类型，则抛出编译时错误
        static_assert(std::is_integral<T>::value, "T must be an integral type");
    }
}

// 主机字节序转网络字节序
template<typename T>
T hostToNetwork(T value)
{
    if constexpr (std::is_integral<T>::value) {
        // 判断整数类型的大小
        if constexpr (sizeof(T) == sizeof(uint32_t)) {
            // 如果 T 的大小是 4 字节，则使用 be32toh 函数
            return static_cast<T>(htobe32(static_cast<uint32_t>(value)));
        } else if constexpr (sizeof(T) == sizeof(uint64_t)) {
            // 如果 T 的大小是 8 字节，则使用 be64toh 函数
            return static_cast<T>(htobe64(static_cast<uint64_t>(value)));
        } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
            // 如果 T 的大小是 2 字节，则使用 be16toh 函数
            return static_cast<T>(htobe16(static_cast<uint16_t>(value)));
        } else {
            // 如果 T 的大小既不是 4 字节也不是 8 字节，则抛出编译时错误
            static_assert(sizeof(T) == sizeof(uint16_t) || sizeof(T) == sizeof(uint32_t) || sizeof(T) == sizeof(uint64_t),
                          "Unsupported integral type size");
        }
    } else {
        // 如果不是整数类型，则抛出编译时错误
        static_assert(std::is_integral<T>::value, "T must be an integral type");
    }
}


}// namespace mrpc

#endif// MRPC_COMMON_UTIL_H