/**
 * @file base_server.h
 * @brief Unix Domain Socket服务端.
 * @author Leopard-C (leopard.c@outlook.com)
 * @version 0.1
 * @date 2023-03-10
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_UDS_BASE_SERVER_H_
#define IC_UDS_BASE_SERVER_H_
#include <functional>
#include <string>
#include <system_error>
#include <sys/un.h>

namespace ic {
namespace uds {

namespace _detail {
class ImplBaseServer;
} // namespace _detail

class BaseServer {
public:
    BaseServer();
    virtual ~BaseServer();

public:
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
     * @brief 接收到完成数据后的回调函数.
     */
    using RequestCallback = std::function<void(
            BaseServer*        base_server,  /* 当前服务器指针 */
            const sockaddr_un& client_addr,  /* 来源客户端地址 */
            int64_t            request_id,   /* 请求ID，由客户端保证每次发送的请求ID是唯一的 */
            const std::string& data          /* 请求数据 */
        )>;
    void set_request_callback(RequestCallback callback);

    /**
     * @brief 服务器是否已停止.
     */
    bool stopped() const;

    const std::string& socket_file() const;
    size_t thread_pool_size() const;

private:
    _detail::ImplBaseServer* impl_{nullptr};
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_BASE_SERVER_H_
