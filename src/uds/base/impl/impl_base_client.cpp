#include "impl_base_client.h"
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util/uds_util.h"
#include "../error_code.h"

namespace ic {
namespace uds {
namespace _detail {

ImplBaseClient::ImplBaseClient() {
    auto now = std::chrono::steady_clock::now();
    curr_request_id_ = now.time_since_epoch().count();
    last_cleanup_time_ = now;
}

ImplBaseClient::~ImplBaseClient() {
    should_stop_ = true;
    int count = 10000;
    while (!stopped_ && --count) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!stopped_) {
        fprintf(stderr, "Background thread of UDS.ImplBaseClient is not quit");
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    if (inited_ && access(client_socket_file_.c_str(), 0) == 0) {
        unlink(client_socket_file_.c_str());
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
}

/**
 * @brief 初始化.
 */
void ImplBaseClient::Init(const std::string& server_socket_file, const std::string& client_socket_file, std::error_code& ec) {
    if (inited_) {
        ec = make_error_code(BaseErrc::ReInitialization);
        return;
    }
    if (server_socket_file.size() >= sizeof(sockaddr_un::sun_path) || client_socket_file.size() >= sizeof(sockaddr_un::sun_path)) {
        ec = make_error_code(BaseErrc::InvalidSocketFile);
        return;
    }

    /* 创建套接字 */
    fd_ = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd_ == -1) {
        ec = make_error_code(BaseErrc::CreateSocketFailed);
        return;
    }

    /* 绑定客户端地址 */
    sockaddr_un client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, client_socket_file.c_str());
    if (access(client_socket_file.c_str(), 0) == 0) {
        /* 如果已存在则删除文件 */
        if (unlink(client_socket_file.c_str()) != 0) {
            ::close(fd_);
            fd_ = -1;
            ec = make_error_code(BaseErrc::BusySocketFile);
            return;
        }
    }
    if (::bind(fd_, (sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        ::close(fd_);
        fd_ = -1;
        ec = make_error_code(BaseErrc::BindFailed);
        return;
    }

    /* 服务端地址 */
    memset(&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sun_family = AF_UNIX;
    strcpy(server_addr_.sun_path, server_socket_file.c_str());

    server_socket_file_ = server_socket_file;
    client_socket_file_ = client_socket_file;
    inited_ = true;
    should_stop_ = false;
    stopped_ = false;

    /* 启动后台线程用于接收数据 */
    std::thread t([this]{
        fd_set read_fds;
        Packet* packet = nullptr;
        while (!should_stop_) {
            if (!packet) {
                packet = new Packet();
            }
            if (util::recv_data(fd_, &read_fds, packet, NULL, NULL)) {
                ProcessResponsePacket(packet);
            }
        }
        if (packet) {
            delete packet;
            packet = nullptr;
        }
        stopped_ = true;
    });
    t.detach();

    ec.clear();
}

/**
 * @brief 仅发送数据，不等待服务器返回响应.
 * 
 * @param  data 待发送的数据
 * @param  ec 错误代码
 * @return 当前请求的ID
 * @note 通过 ec 判断是否成功
 */
int64_t ImplBaseClient::Send(const std::string& data, std::error_code& ec) {
    int64_t request_id = curr_request_id_.fetch_add(1);
    if (!inited_) {
        ec = make_error_code(BaseErrc::NotInitialized);
        return request_id;
    }

    if (!util::send_data(fd_, server_addr_, request_id, data)) {
        ec = make_error_code(BaseErrc::SendFailed);
        return request_id;
    }

    ec.clear();
    return request_id;
}

/**
 * @brief 发送数据，等待服务器返回响应.
 * 
 * @param  data 待发送的数据
 * @param  response 服务器响应数据
 * @param  timeout_ms 超时时间，单位：毫秒
 * @param  ec 错误代码
 * @return 当前请求的ID
 * @note 通过 ec 判断是否成功
 */
int64_t ImplBaseClient::SendRequest(const std::string& data, std::string* response, uint32_t timeout_ms, std::error_code& ec) {
    int64_t request_id = curr_request_id_.fetch_add(1);
    if (!inited_) {
        ec = make_error_code(BaseErrc::NotInitialized);
        return request_id;
    }

    {
        std::lock_guard<std::mutex> lck(mutex_);
        recv_response_ids_.emplace(request_id, std::chrono::steady_clock::now());
    }

    do {
        /* 发送请求 */
        if (!util::send_data(fd_, server_addr_, request_id, data)) {
            ec = make_error_code(BaseErrc::SendFailed);
            break;
        }

        auto timeout_tp = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_ms);
        auto predicate = [this, request_id]{
            return this->prepared_buffers_.find(request_id) != this->prepared_buffers_.end();
        };

        /* 接收响应 */
        std::unique_lock<std::mutex> lck(mutex_);
        if (cv_.wait_until(lck, timeout_tp, predicate)) {
            auto iter = prepared_buffers_.find(request_id);
            if (iter != prepared_buffers_.end()) {
                response->swap(iter->second.first);
                prepared_buffers_.erase(iter);
                ec.clear();
            }
            else {
                // won't get here
                ec = make_error_code(BaseErrc::RecvFailed);
            }
        }
        else {
            ec = make_error_code(BaseErrc::Timeout);
        }
    } while (false);

    {
        std::lock_guard<std::mutex> lck(mutex_);
        recv_response_ids_.erase(request_id);
    }

    return request_id;
}

/**
 * @brief 清理过期的缓存.
 */
void ImplBaseClient::CleanupBuffers(const tp& before) {
    for (auto iter = buffers_.begin(); iter != buffers_.end();/* ++iter*/) {
        for (auto iter2 = iter->second.begin(); iter2 != iter->second.end();/* ++iter2*/) {
            if ((*iter2)->arrive_time < before) {
                iter2 = iter->second.erase(iter2);
            }
            else {
                ++iter2;
            }
        }
        if (iter->second.empty()) {
            iter = buffers_.erase(iter);
        }
        else {
            ++iter;
        }
    }
    for (auto iter = prepared_buffers_.begin(); iter != prepared_buffers_.end();/* ++iter*/) {
        if (iter->second.second < before) {
            iter = prepared_buffers_.erase(iter);
        }
        else {
            ++iter;
        }
    }
    for (auto iter = recv_response_ids_.begin(); iter != recv_response_ids_.end();/* ++iter*/) {
        if (iter->second < before) {
            iter = recv_response_ids_.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

/**
 * @brief 处理接收到的数据(服务端响应).
 */
void ImplBaseClient::ProcessResponsePacket(Packet*& packet) {
    std::lock_guard<std::mutex> lck(mutex_);

    /* 清理60s之前的数据包缓存 */
    auto now = std::chrono::steady_clock::now();
    auto before = now - std::chrono::seconds(60);
    if (last_cleanup_time_ < before) {
        CleanupBuffers(before);
        last_cleanup_time_ = now;
    }

    int64_t id = packet->id;
    uint32_t total = packet->total;
    //printf("recv %ld\n", id);

    /* 是否需要接收响应内容 */
    auto recv_iter = recv_response_ids_.find(id);
    bool need_recv = (recv_iter != recv_response_ids_.end());
    if (!need_recv) {
        return;  /* 丢弃响应内容 */
    }

    if (total <= 1) {
        recv_response_ids_.erase(recv_iter);
        prepared_buffers_.emplace(id, std::make_pair(packet->data, now));
        cv_.notify_all();
    }
    else {
        /* 写入缓冲区 */
        auto iter = buffers_.find(id);
        if (iter == buffers_.end()) {
            iter = buffers_.emplace(id, Packets()).first;
        }
        iter->second.emplace_back(packet);
        packet = nullptr;  // reset packet to nullptr !!!
        /* 所有包已到达 */
        if (iter->second.size() >= total) {
            recv_response_ids_.erase(recv_iter);
            prepared_buffers_.emplace(id, std::make_pair(iter->second.Merge(), now));
            buffers_.erase(iter);
            cv_.notify_all();
        }
    }
}

} // namespace _detail
} // namespace uds
} // namespace ic
