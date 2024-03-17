#ifndef MRPC_NET_CODER_TINYPB_PROTOCOL_H
#define MRPC_NET_CODER_TINYPB_PROTOCOL_H

#include "src/net/coder/abstract_protocol.h"

namespace mrpc
{

struct TinyPBProtocol : public AbstractProtocol
{
    /*    1        4       4        len      4        len   4         4          len           len           4      1  */
    /* [开始符][整包长度][msgid长度][msgid][方法名长度][方法名][错误码][错误信息长度][错误信息][Protobuf 序列化数据][校验和][结束符] */
public:
    static char PB_START;// 开始字符 0x02
    static char PB_END;  // 结束字符 0x03

public:
    int32_t m_pk_len { 0 };          // 整包长度
    int32_t m_msg_id_len { 0 };      // msgid 长度
    int32_t m_method_len { 0 };      // 方法名长度
    int32_t m_error_code { 0 };      // 错误码
    int32_t m_error_info_len { 0 };  // 错误信息长度
    int32_t m_check_sum { 0 };       // 校验和
    std::string m_method_name { "" };// 方法名
    std::string m_error_info { "" }; // 错误信息
    std::string m_pb_data { "" };    // Protobuf 序列化数据

    bool parse_success { false };
};


}// namespace mrpc

#endif//MRPC_NET_CODER_TINYPB_PROTOCOL_H