#ifndef MRPC_COMMON_ERROR_CODE_H
#define MRPC_COMMON_ERROR_CODE_H

#ifndef SYS_ERROR_PERFIX
#define SYS_ERROR_PERFIX(xx) 1000##xx
#endif//SYS_ERROR_PERFIX

#include <cstdint>

const int32_t ERROR_PEER_CLOSE = SYS_ERROR_PERFIX(0000);        // 连接时对端关闭
const int32_t ERROR_FAILED_CONNECT = SYS_ERROR_PERFIX(0001);    // 链接失败
const int32_t ERROR_FAILED_GET_REPLY = SYS_ERROR_PERFIX(0002);  // 获取回包失败
const int32_t ERROR_FAILED_DESERIALIZE = SYS_ERROR_PERFIX(0003);//反序列化失败
const int32_t ERROR_FAILED_SERIALIZE = SYS_ERROR_PERFIX(0004);  // 序列化失败
const int32_t ERROR_FAILED_ENDODE = SYS_ERROR_PERFIX(0005);     // encode failed
const int32_t ERROR_FAILED_DECODE = SYS_ERROR_PERFIX(0006);     // decode failed
const int32_t ERROR_RPC_CALL_TIMEOUT = SYS_ERROR_PERFIX(0007);  // rpc调用超时
const int32_t ERROR_SERVICE_NOT_FOUND = SYS_ERROR_PERFIX(0008); // rpc service 不存在
const int32_t ERROR_METHOD_NOT_FOUND = SYS_ERROR_PERFIX(0009);  // rpc service.method 不存在
const int32_t ERROR_PARSE_SERVICE_NAME = SYS_ERROR_PERFIX(0010);// service name 解析失败

#endif//MRPC_COMMON_ERROR_CODE_H