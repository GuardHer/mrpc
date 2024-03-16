#ifndef MRPC_NET_RPC_RPC_CONTROLLER_H
#define MRPC_NET_RPC_RPC_CONTROLLER_H

#include "src/net/tcp/net_addr.h"
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>

namespace mrpc
{

class RpcController : public google::protobuf::RpcController
{
public:
    RpcController();
    ~RpcController();

public:
    /// @brief 是否失败
    /// @return m_is_failed
    bool Failed() const override;

    /// @brief 是否取消
    /// @return m_is_cancle
    bool IsCanceled() const override;

    // 重置
    void Reset() override;

    /// @brief 取消
    void StartCancel() override;

    /// @brief 设置错误信息
    /// @param reason 错误信息
    void SetFailed(const std::string &reason) override;

    /// @brief rpc 调用被取消时通知客户端
    /// @param callback 回调函数
    void NotifyOnCancel(google::protobuf::Closure *callback) override;

    /// @brief 获取错误信息
    /// @return m_error_info
    std::string ErrorText() const override;

    /// @brief 设置错误信息和错误码
    /// @param error_code
    /// @param error_info
    void SetError(int32_t error_code, const std::string &error_info);

    /// @brief 获取错误码
    /// @return m_error_code
    int32_t GetErrorCode() const;

    /// @brief 获取错误信息
    /// @return m_error_info
    std::string GetErrorInfo() const;

    /// @brief 设置msg_id
    /// @param msg_id
    void SetMsgId(const std::string &msg_id);

    /// @brief 获取 msg_id
    /// @return m_msg_id
    std::string GetMsgId() const;

    /// @brief 设置本地地址
    /// @param addr
    void SetLocalAddr(NetAddr::s_ptr addr);

    /// @brief 设置对端地址
    /// @param addr
    void SetPeerAddr(NetAddr::s_ptr addr);

    /// @brief 获取本地地址
    /// @return m_local_addr
    NetAddr::s_ptr GetLocalAddr() const;

    /// @brief 获取对端地址
    /// @return m_peer_addr
    NetAddr::s_ptr GetPeerAddr() const;

    /// @brief 设置超时
    /// @param timeout
    void SetTimeout(int32_t timeout);

    /// @brief 获取超时
    /// @return m_timeout
    int32_t GetTimeout() const;


private:
    int32_t m_error_code{0};    // 错误码
    int32_t m_timeout{1000};    // 超时 ms
    std::string m_error_info;   // 错误信息
    std::string m_msg_id;       // 请求码
    bool m_is_failed{false};    // 是否失败
    bool m_is_cancle{false};    // 是否取消
    NetAddr::s_ptr m_loacl_addr;// 本地地址
    NetAddr::s_ptr m_peer_addr; // 对端地址
};

}// namespace mrpc

#endif//MRPC_NET_RPC_RPC_CONTROLLER_H