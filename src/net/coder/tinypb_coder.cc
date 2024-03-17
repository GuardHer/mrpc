#include "src/net/coder/tinypb_coder.h"

#include "src/common/log.h"
#include "src/common/util.h"
#include <iostream>
#include <sstream>
namespace mrpc
{

void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr &out_buffer)
{
    for (auto &message: messages) {
        std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(message);
        int len                             = 0;
        // if (!checkPack(msg)) {
        //     LOG_INFO << "! checkPack";
        //     continue;
        // }
        std::string buf = encodeTinyPB(msg, len);
        if (!buf.empty() && len != 0) {
            out_buffer->wirteToBuffer(buf);
        }
    }
}

void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer)
{
    while (1) {
        // 遍历buffer, 找到 PB_START, 解析出整包的长度, 然后得到 PB_END
        std::vector<char> tmp = buffer->buffer();
        int start_index       = buffer->readIndex();
        int end_index         = -1;

        int pk_len      = 0;
        int i           = 0;
        bool parse_succ = false;
        for (i = start_index; i < buffer->writeIndex(); i++) {
            if (tmp[i] == TinyPBProtocol::PB_START) {

                pk_len = networkToHost32(&tmp[i + 1]);
                int j  = i + pk_len - 1;

                LOG_INFO << "get pk_len = " << pk_len << ", i: " << i << ", j: " << j;

                // 包不完整
                if (j >= buffer->writeIndex()) {
                    continue;
                }

                if (tmp[j] == TinyPBProtocol::PB_END) {
                    start_index = i;
                    end_index   = j;
                    parse_succ  = true;
                    break;
                }
            }
        }

        if (i >= buffer->writeIndex()) {
            LOG_DEBUG << "decode end, read all buffer data!";
            return;
        }

        if (parse_succ) {
            if (pk_len < 26) {
                LOG_ERROR << "parse error, pk_len: " << pk_len << " < 26";
                return;
            }
            if (buffer->readAble() < pk_len) return;

            auto message    = std::make_shared<TinyPBProtocol>();
            std::string str = buffer->peekAsString(start_index, end_index);
            buffer->moveReadIndex(end_index - start_index + 1);

            TcpBuffer buf(str);
            buf.readAsString(1);                                                    // PB_START
            message->m_pk_len = pk_len;                                             // m_pk_len
            buf.readInt<int32_t>();                                                 //
            message->m_msg_id_len     = buf.readInt<int32_t>();                     // m_msg_id_len
            message->m_msg_id         = buf.readAsString(message->m_msg_id_len);    // m_msg_id
            message->m_method_len     = buf.readInt<int32_t>();                     // m_method_len
            message->m_method_name    = buf.readAsString(message->m_method_len);    // m_method_name
            message->m_error_code     = buf.readInt<int32_t>();                     // m_error_code
            message->m_error_info_len = buf.readInt<int32_t>();                     // m_error_info_len
            message->m_error_info     = buf.readAsString(message->m_error_info_len);// m_error_info
            int32_t pb_data_len       = message->m_pk_len - message->m_msg_id_len - message->m_method_len - message->m_error_info_len - 26;
            message->m_pb_data        = buf.readAsString(pb_data_len);// m_pb_data
            message->m_check_sum      = buf.readInt<int32_t>();       // m_check_sum
            buf.readAsString(1);                                      // PB_END

            message->parse_success = true;

            out_messages.push_back(message);
        }
    }
}

std::string TinyPBCoder::encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int &len)
{
    if (message->m_msg_id.empty()) {
        message->m_msg_id     = "123456789";
        message->m_msg_id_len = static_cast<int32_t>(message->m_msg_id.length());
    }
    int32_t pk_len = 26 + message->m_error_info_len + message->m_method_len + message->m_msg_id_len + message->m_pb_data.length();

    TcpBuffer buffer(pk_len);
    buffer.wirteToBuffer(&TinyPBProtocol::PB_START, 1); // PB_START
    buffer.writeInt<int32_t>(pk_len);                   // pk_len
    buffer.writeInt<int32_t>(message->m_msg_id_len);    // m_msg_id_len
    buffer.wirteToBuffer(message->m_msg_id);            // m_msg_id
    buffer.writeInt<int32_t>(message->m_method_len);    // m_method_len
    buffer.wirteToBuffer(message->m_method_name);       // m_method_name
    buffer.writeInt<int32_t>(message->m_error_code);    // m_error_code
    buffer.writeInt<int32_t>(message->m_error_info_len);// m_error_info_len
    buffer.wirteToBuffer(message->m_error_info);        // m_error_info
    buffer.wirteToBuffer(message->m_pb_data);           // m_pb_data
    buffer.writeInt<int32_t>(message->m_check_sum);     // m_check_sum
    buffer.wirteToBuffer(&TinyPBProtocol::PB_END, 1);   // PB_START

    std::string result = buffer.readAllAsString();
    LOG_DEBUG << "result len: " << result.length() << ", pk_len: " << pk_len;
    len = pk_len;
    return result;
}

bool TinyPBCoder::checkPack(const std::shared_ptr<TinyPBProtocol> &message)
{
    int32_t msg_id_len     = message->m_msg_id_len;
    int32_t method_len     = message->m_method_len;
    int32_t error_info_len = message->m_error_info_len;
    int32_t pk_len         = 26 + msg_id_len + method_len + error_info_len + message->m_pb_data.length();


    if (msg_id_len != static_cast<int32_t>(message->m_msg_id.length())) return false;
    if (method_len != static_cast<int32_t>(message->m_method_name.length())) return false;
    if (error_info_len != static_cast<int32_t>(message->m_error_info.length())) return false;
    if (pk_len != message->m_pk_len) return false;


    return true;
}


}// namespace mrpc