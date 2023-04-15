#include "request.h"

namespace ic {
namespace uds {

std::string Request::Serialize(bool clear_body/* = false*/) {
    json_[":path"] = path_;
    return Message::Serialize(clear_body);
}

bool Request::Deserialize(const std::string& data) {
    if (!Message::Deserialize(data)) {
        return false;
    }
    /* 请求路径 */
    if (!json_[":path"].isString()) {
        return false;
    }
    path_ = json_[":path"].asString();
    if (path_.empty()) {
        return false;
    }
    return true;
}

} // namespace uds
} // namespace ic
