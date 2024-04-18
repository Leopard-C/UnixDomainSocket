#include "impl_base_server.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "thread/static_thread_pool.h"
#include "util/uds_util.h"
#include "../base_server.h"
#include "../error_code.h"

namespace ic {
namespace uds {
namespace _detail {

ImplBaseServer::ImplBaseServer(BaseServer* base_server)
    : base_server_(base_server), last_cleanup_time_(std::chrono::steady_clock::now())
{
}

ImplBaseServer::~ImplBaseServer() {
    Stop();
    int count = 10000;
    while (!stopped_ && --count) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!stopped_) {
        fprintf(stderr, "UDS.Server stop failed in 10 seconds");
    }
    if (thread_pool_) {
        thread_pool_->Wait();
        delete thread_pool_;
        thread_pool_ = nullptr;
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    if (inited_ && access(socket_file_.c_str(), 0) == 0) {
        unlink(socket_file_.c_str());
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
}

/**
 * @brief 初始化.
 */
void ImplBaseServer::Init(const std::string& socket_file, size_t thread_pool_size, std::error_code& ec) {
    if (inited_) {
        ec = make_error_code(BaseErrc::ReInitialization);
        return;
    }
    if (socket_file.size() >= sizeof(sockaddr_un::sun_path)) {
        ec = make_error_code(BaseErrc::InvalidSocketFile);
        return;
    }

    if (thread_pool_size == 0) {
        thread_pool_size = std::thread::hardware_concurrency();
    }

    /* 创建套接字 */
    fd_ = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd_ == -1) {
        ec = make_error_code(BaseErrc::CreateSocketFailed);
        return;
    }

    /* 绑定地址 */
    sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_file.c_str());
    if (access(socket_file.c_str(), 0) == 0) {
        /* 如果已存在则删除文件 */
        if (unlink(socket_file.c_str()) != 0) {
            ::close(fd_);
            fd_ = -1;
            ec = make_error_code(BaseErrc::BusySocketFile);
            return;
        }
    }
    if (bind(fd_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ::close(fd_);
        fd_ = -1;
        ec = make_error_code(BaseErrc::BindFailed);
        return;
    }

    /* 创建线程池 */
    thread_pool_ = new StaticThreadPool(thread_pool_size);

    socket_file_ = socket_file;
    thread_pool_size_ = thread_pool_size;
    inited_ = true;
    ec.clear();
}

/**
 * @brief 启动服务器.
 */
void ImplBaseServer::Start() {
    if (!inited_ || !stopped_) {
        return;
    }
    should_stop_ = false;
    stopped_ = false;

    struct sockaddr_un client_addr;
    socklen_t client_addr_length = sizeof(client_addr);
    fd_set read_fds;
    Packet* packet = nullptr;

    while (!should_stop_) {
        if (!packet) {
            packet = new Packet();
        }
        client_addr_length = sizeof(client_addr);
        memset(&client_addr, 0, client_addr_length);
        if (util::recv_data(fd_, &read_fds, packet, &client_addr, &client_addr_length)) {
            ProcessRequestPacket(client_addr, packet);
        }
    }
    if (packet) {
        delete packet;
        packet = nullptr;
    }

    stopped_ = true;
}

/**
 * @brief 停止服务器.
 */
void ImplBaseServer::Stop() {
    should_stop_ = true;
}

/**
 * @brief 返回给客户端响应数据.
 * 
 * @param client_addr 客户端地址
 * @param request_id 客户端的请求ID
 * @param data 响应内容
 */
bool ImplBaseServer::SendResponse(const sockaddr_un& client_addr, int64_t request_id, const std::string& data) {
    return util::send_data(fd_, client_addr, request_id, data);
}

/**
 * @brief 清理缓存中过期的数据包.
 */
void ImplBaseServer::CleanupBuffers(const tp& before) {
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
}

/**
 * @brief 处理接收到的数据包.
 */
void ImplBaseServer::ProcessRequestPacket(const sockaddr_un& client_addr, Packet*& packet) {
    /* 清理60s之前的数据包 */
    auto now = std::chrono::steady_clock::now();
    auto before = now - std::chrono::seconds(60);
    if (last_cleanup_time_ < before) {
        CleanupBuffers(before);
        last_cleanup_time_ = now;
    }

    int64_t id = packet->id;
    uint32_t total = packet->total;

    //count_++;
    //printf("%d %ld\n", count_, id);

    if (total <= 1) {
        thread_pool_->Enqueue([this, client_addr, id, data = packet->data]{
            if (this->request_callback_) {
                this->request_callback_(this->base_server_, client_addr, id, data);
            }
        });
    }
    else {
        /* 写入缓冲区 */
        auto key = std::make_pair<std::string, uint32_t>(client_addr.sun_path, id);
        auto iter = buffers_.find(key);
        if (iter == buffers_.end()) {
            iter = buffers_.emplace(key, Packets()).first;
        }
        iter->second.emplace_back(packet);
        packet = nullptr;  // reset packet to nullptr !!!
        /* 所有包已到达 */
        if (iter->second.size() >= total) {
            thread_pool_->Enqueue([this, client_addr, id, data = iter->second.Merge()]{
                if (this->request_callback_) {
                    this->request_callback_(this->base_server_, client_addr, id, data);
                }
            });
            buffers_.erase(iter);
        }
    }
}

} // namespace _detail
} // namespace uds
} // namespace ic
