/**
 * @file router.h
 * @brief 路由.
 * @author Leopard-C (leopard.c@outlook.com)
 * @version 0.1
 * @date 2023-03-12
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_UDS_JSON_ROUTER_H_
#define IC_UDS_JSON_ROUTER_H_
#include <functional>
#include <map>
#include <string>

namespace ic {
namespace uds {

class Request;
class Response;
class Server;

using RequestHandler = std::function<void(Request& req, Response& res)>;

class Route {
public:
    Route() = default;
    Route(const std::string& path, RequestHandler handler)
        : path(path), handler(handler) {}
    Route(const std::string& path, const std::string& description, RequestHandler handler)
        : path(path), description(description), handler(handler) {}
    RequestHandler handler;
    std::string path;
    std::string description;
};

class Router {
public:
    Router(Server* server = nullptr);
    ~Router();

    /**
     * @brief 添加路由.
     * 
     * @retval true 添加成功
     * @retval false 添加失败，路由已存在
     */
    bool AddRoute(const std::string& path, const std::string& description, RequestHandler handler);

    /**
     * @brief 添加路由.
     * 
     * @retval true 添加成功
     * @retval false 添加失败，路由已存在
     */
    bool AddRoute(const std::string& path, RequestHandler handler);

    void set_bad_request_handler(RequestHandler handler) { bad_request_handler_ = handler; }
    void set_invalid_path_handler(RequestHandler handler) { invalid_path_handler_ = handler; }

public:
    void HandleRequest(Request& req, Response& res);
    void HandleBadRequest(Request& req, Response& res);
    void HandleInvalidPath(Request& req, Response& res);

private:
    Server* svr_{nullptr};
    RequestHandler bad_request_handler_;
    RequestHandler invalid_path_handler_;
    std::map<std::string, Route*> routes_;
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_JSON_ROUTER_H_
