/**
 * @file impl_base_client.h
 * @brief Unix Domain Socket客户端实现类.
 * @author Leopard-C (leopard.c@outlook.com)
 * @version 0.1
 * @date 2023-03-11
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_UDS_BASE_IMPL_BASE_CLIENT_H_
#define IC_UDS_BASE_IMPL_BASE_CLIENT_H_
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <system_error>
#include <sys/un.h>
#include "uds_packet.h"

namespace ic {
namespace uds {
namespace _detail {

using tp = std::chrono::steady_clock::time_point;

/**
 * @brief BaseClient的实现类.
 */
class ImplBaseClient {
public:
    ImplBaseClient();
    ~ImplBaseClient();

    /**
     * @brief 初始化.
     */
    void Init(const std::string& server_socket_file, const std::string& client_socket_file, std::error_code& ec);

    /**
     * @brief 仅发送数据，不等待服务器返回响应.
     * 
     * @param  data 待发送的数据
     * @param  ec 错误代码
     * @return 当前请求的ID
     * @note 通过 ec 判断是否成功
     */
    int64_t Send(const std::string& data, std::error_code& ec);

    /**
     * @brief 发送数据，等待服务器返回响应.
     * 
     * @param  data 待发送的数据
     * @param  response 服务器响应数据
     * @param  timeout_ms 超时时间，单位：毫秒
     * @param  ec 错误代码
     * @return 当前请求的ID
     * @note 通过 ec 判断是否成功
     */
    int64_t SendRequest(const std::string& data, std::string* response, uint32_t timeout_ms, std::error_code& ec);

    const std::string& server_socket_file() const { return server_socket_file_; }
    const std::string& client_socket_file() const { return client_socket_file_; }

private:
    void CleanupBuffers(const tp& before);
    void ProcessResponsePacket(Packet*& packet);

private:
    bool inited_ = false;
    bool should_stop_ = false;
    bool stopped_ = true;

    int fd_ = -1;
    std::string server_socket_file_;
    std::string client_socket_file_;

    std::atomic_int64_t curr_request_id_;
    sockaddr_un server_addr_;

    std::mutex mutex_;
    std::condition_variable cv_;

    /* 上次清理缓存的时间 */
    tp last_cleanup_time_;

    /* 正在接收中的缓冲区(分包) */
    std::map<int64_t, Packets> buffers_;

    /* 接收完成的缓冲区(已接收完成并组包) */
    std::map<int64_t, std::pair<std::string, tp>> prepared_buffers_;

    /* 需要接收响应内容的ID */
    std::map<int64_t, tp> recv_response_ids_;
};

} // namespace _detail
} // namespace uds
} // namespace ic

#endif // IC_UDS_BASE_IMPL_BASE_CLIENT_H_
