/**
 * @file base_client.h
 * @brief Unix Domain Socket客户端.
 * @author Leopard-C (leopard.c@outlook.com)
 * @version 0.1
 * @date 2023-03-11
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_UDS_BASE_CLIENT_H_
#define IC_UDS_BASE_CLIENT_H_
#include <string>
#include <system_error>
#include <sys/un.h>

namespace ic {
namespace uds {

namespace _detail {
class ImplBaseClient;
} // namespace _detail

class BaseClient {
public:
    BaseClient();
    virtual ~BaseClient();

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

    const std::string& server_socket_file() const;
    const std::string& client_socket_file() const;

private:
    _detail::ImplBaseClient* impl_{nullptr};
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_BASE_CLIENT_H_
