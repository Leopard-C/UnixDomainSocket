#include "server.h"
#include "request.h"
#include "response.h"
#include "router.h"

namespace ic {
namespace uds {

Server::Server() {
    router_ = new Router();
    this->set_request_callback([](BaseServer* base_server, const sockaddr_un& client_addr, int64_t request_id, const std::string& data){
        Server* server = dynamic_cast<Server*>(base_server);
        Request req(server, &client_addr, request_id);
        Response res(request_id);
        if (req.Deserialize(data)) {
            server->router_->HandleRequest(req, res);
        }
        else {
            server->router_->HandleBadRequest(req, res);
        }
        server->SendResponse(client_addr, request_id, res.Serialize());
    });
}

Server::~Server() {
    if (router_) {
        delete router_;
        router_ = nullptr;
    }
}

} // namespace uds
} // namespace ic
