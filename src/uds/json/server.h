#ifndef IC_UDS_JSON_SERVER_H_
#define IC_UDS_JSON_SERVER_H_
#include "../base/base_server.h"

namespace ic {
namespace uds {

class Router;

class Server : public BaseServer {
public:
    Server();
    ~Server();

    Router* router() { return router_; }

private:
    Router* router_;
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_JSON_SERVER_H_
