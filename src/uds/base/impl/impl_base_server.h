/**
 * @file impl_base_server.h
 * @brief Unix Domain Socket服务端实现类.
 * @author Leopard-C (leopard.c@outlook.com)
 * @version 0.1
 * @date 2023-03-11
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_UDS_IMPL_BASE_SERVER_H_
#define IC_UDS_IMPL_BASE_SERVER_H_
#include <functional>
#include <map>
#include <string>
#include <system_error>
#include <sys/un.h>
#include "uds_packet.h"

namespace ic {
namespace uds {

class BaseServer;
class StaticThreadPool;

namespace _detail {

using tp = std::chrono::steady_clock::time_point;

/**
 * @brief BaseServer的实现类.
 */
class ImplBaseServer {
public:
    ImplBaseServer(BaseServer* server);
    ~ImplBaseServer();

    /**
     * @brief 初始化.
     */
    void Init(const std::string& socket_file, size_t thread_pool_size, std::error_code& ec);

    /**
     * @brief 启动服务器.
     * 
     * @details 阻塞，直到调用Stop停止服务器.
     */
    void Start();

    /**
     * @brief 停止服务器.
     * 
     * @details 非阻塞，仅仅是通知服务器停止接收数据，准备停止.
     */
    void Stop();

    /**
     * @brief 返回给客户端响应数据.
     * 
     * @param client_addr 客户端地址
     * @param request_id 客户端的请求ID
     * @param data 响应内容
     */
    bool SendResponse(const sockaddr_un& client_addr, int64_t request_id, const std::string& data);

    /**
     * @brief 收到请求后的回调函数.
     * 
     * @details 如果需要发送响应给客户端，可以调用`server->SendResponse()`方法.
     * 
     * @param server 当前服务器指针
     * @param client_addr 请求方地址
     * @param request_id 请求ID(客户端指定)
     * @param data 请求内容
     */
    using RequestCallback = std::function<void(
        BaseServer* server, const sockaddr_un& client_addr, int64_t request_id, const std::string& data
    )>;
    void set_request_callback(RequestCallback callback) { request_callback_ = callback; }

    const std::string& socket_file() const { return socket_file_; }
    size_t thread_pool_size() const { return thread_pool_size_; }

    bool stopped() const { return stopped_; }

private:
    void CleanupBuffers(const tp& before);
    void ProcessRequestPacket(const sockaddr_un& client_addr, Packet*& packet);

private:
    bool inited_ = false;
    bool should_stop_ = false;
    bool stopped_ = true;

    int fd_ = -1;
    size_t thread_pool_size_ = 1;
    std::string socket_file_;

    BaseServer* base_server_;
    StaticThreadPool* thread_pool_ = nullptr;

    /* 接收到请求后的回调函数 */
    RequestCallback request_callback_;

    /* 上次清理缓存的时间 */
    tp last_cleanup_time_;

    /* 数据包缓存 */
    std::map<std::pair<std::string, uint32_t>, Packets> buffers_;

    int count_ = 0;
};

} // namespace _detail
} // namespace uds
} // namespace ic

#endif // IC_UDS_IMPL_BASE_SERVER_H_
