#include "client.h"
#include "../base/error_code.h"

namespace ic {
namespace uds {

Response Client::SendRequest(Request& req, unsigned int timeout_ms/* = 10000*/) {
    std::error_code ec;
    std::string response_data;
    int64_t id = BaseClient::SendRequest(req.Serialize(true), &response_data, timeout_ms, ec);
    Response res(id);
    if (!ec) {
        if (!res.Deserialize(response_data)) {
            res.set_status(Response::Status::BadResponse);
        }
    }
    else if (ec == BaseErrc::SendFailed) {
        res.set_status(Response::Status::SendFailed);
    }
    else if (ec == BaseErrc::RecvFailed) {
        res.set_status(Response::Status::RecvFailed);
    }
    else if (ec == BaseErrc::Timeout) {
        res.set_status(Response::Status::Timeout);
    }
    else if (ec == BaseErrc::NotInitialized) {
        res.set_status(Response::Status::NotInitialized);
    }
    else {
        // wont' get here
        res.set_status(Response::Status::UnknownError);
    }
    return res;
}

} // namespace uds
} // namespace ic
