#ifndef IC_UDS_JSON_REQUEST_H_
#define IC_UDS_JSON_REQUEST_H_
#include <chrono>
#include <sys/un.h>
#include "message.h"

namespace ic {
namespace uds {

class Route;
class Router;
class Server;
class Client;

class Request : public Message {
public:
    friend class Router;
    friend class Server;
    friend class Client;
    using tp = std::chrono::system_clock::time_point;

    Request() = default;
    Request(const std::string& path) : path_(path) {}
    Request(Server* svr, const sockaddr_un* client_addr, int64_t id)
        : Message(id), svr_(svr), client_addr_(client_addr) {}

public:
    const std::string& path() const { return path_; }
    void set_path(const std::string& path) { path_ = path; }

    const Route* route() const { return route_; }

    Server* svr() const { return svr_; }
    const sockaddr_un* client_addr() const { return client_addr_; }

    const tp& timepoint() const { return timepoint_; }
    void set_timepoint(const tp& timepoint) { timepoint_ = timepoint; }

protected:
    /**
     * @brief 序列化为字符串，用于发送.
     */
    std::string Serialize(bool clear_body = false);

    /**
     * @brief 反序列化，解析字符串数据.
     */
    bool Deserialize(const std::string& data);

private:
    Server* svr_{nullptr};
    const sockaddr_un* client_addr_{nullptr};

    /**
     * @brief 当前请求命中的路由.
     */
    const Route* route_{nullptr};

    /**
     * @brief 请求路径，如/User/GetInfo
     */
    std::string path_;

    /**
     * @brief 请求时间戳.
     */
    tp timepoint_{std::chrono::system_clock::now()};
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_JSON_REQUEST_H_
