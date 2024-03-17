#include "src/net/rpc/rpc_controller.h"

namespace mrpc
{

RpcController::RpcController()
{
}

RpcController::~RpcController()
{
}

bool RpcController::Failed() const
{
    return m_is_failed;
}

bool RpcController::IsCanceled() const
{
    return m_is_cancle;
}

void RpcController::Reset()
{
    m_error_code = 0;
    m_timeout    = 1000;
    m_error_info = "";
    m_msg_id     = "";
    m_is_failed  = false;
    m_is_cancle  = false;
    m_loacl_addr = nullptr;
    m_peer_addr  = nullptr;
}

void RpcController::StartCancel()
{
    m_is_cancle = true;
}

void RpcController::SetFailed(const std::string &reason)
{
    m_error_info = reason;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure *callback)
{
    if (callback) {
        callback->Run();
    }
}

std::string RpcController::ErrorText() const
{
    return m_error_info;
}

void RpcController::SetError(int32_t error_code, const std::string &error_info)
{
    m_error_code = error_code;
    m_error_info = error_info;
    m_is_failed  = true;
}

int32_t RpcController::GetErrorCode() const
{
    return m_error_code;
}

std::string RpcController::GetErrorInfo() const
{
    return m_error_info;
}

void RpcController::SetMsgId(const std::string &msg_id)
{
    m_msg_id = msg_id;
}

std::string RpcController::GetMsgId() const
{
    return m_msg_id;
}

void RpcController::SetLocalAddr(NetAddr::s_ptr addr)
{
    m_loacl_addr = addr;
}

void RpcController::SetPeerAddr(NetAddr::s_ptr addr)
{
    m_peer_addr = addr;
}

NetAddr::s_ptr RpcController::GetLocalAddr() const
{
    return m_loacl_addr;
}

NetAddr::s_ptr RpcController::GetPeerAddr() const
{
    return m_peer_addr;
}

void RpcController::SetTimeout(int32_t timeout)
{
    m_timeout = timeout;
}

int32_t RpcController::GetTimeout() const
{
    return m_timeout;
}

}// namespace mrpc