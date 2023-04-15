#ifndef IC_UDS_JSON_CLIENT_H_
#define IC_UDS_JSON_CLIENT_H_
#include "request.h"
#include "response.h"
#include "../base/base_client.h"

namespace ic {
namespace uds {

class Client : public BaseClient {
public:
    Response SendRequest(Request& req, unsigned int timeout_ms = 10000);
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_JSON_CLIENT_H_
