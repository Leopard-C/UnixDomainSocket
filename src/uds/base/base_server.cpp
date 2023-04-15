#include "base_server.h"
#include "impl/impl_base_server.h"

namespace ic {
namespace uds {

BaseServer::BaseServer() {
    impl_ = new _detail::ImplBaseServer(this);
}

BaseServer::~BaseServer() {
    delete impl_;
    impl_ = nullptr;
}

void BaseServer::Init(const std::string& socket_file, size_t thread_pool_size, std::error_code& ec) {
    impl_->Init(socket_file, thread_pool_size, ec);
}

void BaseServer::Start() {
    impl_->Start();
}

void BaseServer::Stop() {
    impl_->Stop();
}

bool BaseServer::SendResponse(const sockaddr_un& client_addr, int64_t request_id, const std::string& data) {
    return impl_->SendResponse(client_addr, request_id, data);
}

void BaseServer::set_request_callback(RequestCallback callback) {
    impl_->set_request_callback(callback);
}

const std::string& BaseServer::socket_file() const {
    return impl_->socket_file();
}

size_t BaseServer::thread_pool_size() const {
    return impl_->thread_pool_size();
}

bool BaseServer::stopped() const {
    return impl_->stopped();
}

} // namespace uds
} // namespace ic
