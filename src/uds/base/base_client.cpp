#include "base_client.h"
#include "impl/impl_base_client.h"

namespace ic {
namespace uds {

BaseClient::BaseClient() {
    impl_ = new _detail::ImplBaseClient();
}

BaseClient::~BaseClient() {
    delete impl_;
    impl_ = nullptr;
}

void BaseClient::Init(const std::string& server_socket_file, const std::string& client_socket_file, std::error_code& ec) {
    impl_->Init(server_socket_file, client_socket_file, ec);
}

int64_t BaseClient::Send(const std::string& data, std::error_code& ec) {
    return impl_->Send(data, ec);
}

int64_t BaseClient::SendRequest(const std::string& data, std::string* response, uint32_t timeout_ms, std::error_code& ec) {
    return impl_->SendRequest(data, response, timeout_ms, ec);
}

const std::string& BaseClient::server_socket_file() const {
    return impl_->server_socket_file();
}

const std::string& BaseClient::client_socket_file() const {
    return impl_->client_socket_file();
}

} // namespace uds
} // namespace ic
