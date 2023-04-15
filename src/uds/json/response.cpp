#include "response.h"

namespace ic {
namespace uds {

std::string Response::Serialize(bool clear_body/* = false*/) {
    json_[":status"] = (int)status_;
    return Message::Serialize(clear_body);
}

bool Response::Deserialize(const std::string& data) {
    if (!Message::Deserialize(data)) {
        return false;
    }
    int status_value = json_[":status"].asInt();
    switch (status_value) {
        case (int)Status::Success:
        case (int)Status::BadRequest:
        case (int)Status::BadResponse:
        case (int)Status::InvalidPath:
        case (int)Status::NotInitialized:
        case (int)Status::SendFailed:
        case (int)Status::RecvFailed:
        case (int)Status::Timeout:
        case (int)Status::UnknownError:
            status_ = Status(status_value);
            break;
        default:
            status_ = Status::BadResponse;
            break;
    }
    return true;
}

const char* Response::message() const {
    switch (status_) {
        case Status::Success:      return "Success";
        case Status::BadRequest:   return "Bad request";
        case Status::BadResponse:  return "Bad response";
        case Status::InvalidPath:  return "Invalid request path";
        case Status::NotInitialized: return "Not initialized";
        case Status::SendFailed:   return "Send request failed";
        case Status::RecvFailed:   return "Receive response failed";
        case Status::Timeout:      return "Receive response timeout";
        case Status::UnknownError:
        default:                   return "(not recognized error)";
    }
}

} // namespace uds
} // namespace ic
