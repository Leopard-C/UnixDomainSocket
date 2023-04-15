#include "router.h"
#include "request.h"
#include "response.h"

namespace ic {
namespace uds {

Router::Router(Server* server/* = nullptr*/)
    : svr_(server)
{
}

Router::~Router() {
    for (auto iter = routes_.begin(); iter != routes_.end(); ++iter) {
        delete iter->second;
    }
}

bool Router::AddRoute(const std::string& path, const std::string& description, RequestHandler handler) {
    auto iter = routes_.find(path);
    if (iter != routes_.end()) {
        fprintf(stderr, "Duplicate route. path=%s\n", path.c_str());
        return false;
    }
    Route* route = new Route(path, description, handler);
    routes_.emplace(path, route);
    return true;
}

bool Router::AddRoute(const std::string& path, RequestHandler handler) {
    return AddRoute(path, "", handler);
}

void Router::HandleRequest(Request& req, Response& res) {
    auto iter = routes_.find(req.path());
    if (iter != routes_.end()) {
        req.route_ = iter->second;
        iter->second->handler(req, res);
        res.set_status(Response::Status::Success);
    }
    else {
        HandleInvalidPath(req, res);
    }
}

void Router::HandleBadRequest(Request& req, Response& res) {
    if (bad_request_handler_) {
        bad_request_handler_(req, res);
    }
    res.set_status(Response::Status::BadRequest);
}

void Router::HandleInvalidPath(Request& req, Response& res) {
    if (invalid_path_handler_) {
        invalid_path_handler_(req, res);
    }
    res.set_status(Response::Status::InvalidPath);
}

} // namespace uds
} // namespace ic
