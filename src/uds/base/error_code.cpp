#include "error_code.h"
#include <string>

namespace ic {
namespace uds {

namespace {

struct BaseErrcCategory : std::error_category {
    const char* name() const noexcept override {
        return "UdsBase";
    }

    std::string message(int ev) const override {
        switch (static_cast<BaseErrc>(ev)) {
            case BaseErrc::ReInitialization:   return "Repeat initialization";
            case BaseErrc::CreateSocketFailed: return "Create socket failed";
            case BaseErrc::BusySocketFile:     return "Socket file is busy";
            case BaseErrc::BindFailed:         return "Bind socket file failed";
            case BaseErrc::NotInitialized:     return "Not initialized";
            case BaseErrc::SendFailed:         return "Send data failed";
            case BaseErrc::RecvFailed:         return "Receive data failed";
            case BaseErrc::Timeout:            return "Receive data timeout";
            default:                           return "(unrecognized error)";
        }
    }
};

const BaseErrcCategory theUdsBaseErrCategory{};

} // anonymouse namespace

std::error_code make_error_code(BaseErrc ec) {
    return { static_cast<int>(ec), theUdsBaseErrCategory };
}

} // namespace uds
} // namespace ic
