#include <chrono>
#include <fstream>
#include <memory>
#include <stdio.h>
#include <signal.h>
#include "uds/base/base_server.h"

std::shared_ptr<ic::uds::BaseServer> g_server;
const char* socket_file = "/dev/shm/.file_receiver.sock";
const size_t thread_pool_size = 4;

/**
 * @brief 捕获Ctrl+C事件
 */
void CatchCtrlC(int sig) {
    if (g_server) {
        g_server->Stop();
    }
}

int main() {
    signal(SIGINT, CatchCtrlC);

    /*
     * 1. 创建服务器
     */
    g_server = std::make_shared<ic::uds::BaseServer>();

    /*
     * 2. 初始化
     */
    std::error_code ec;
    g_server->Init(socket_file, thread_pool_size, ec);
    if (ec) {
        printf("[Error] UDS.BaseServer init failed. %s\n", ec.message().c_str());
        return 1;
    }

    /*
     * 3. 接收到请求后的回调函数
     */
    g_server->set_request_callback([](ic::uds::BaseServer* server, const sockaddr_un& client_addr, int64_t request_id, const std::string& data){
        printf(
            "client: %s\nrequest_id: %ld\ndata length: %lu\n\n",
            client_addr.sun_path, request_id, data.length()
        );
        std::string filename = "/dev/shm/recv_" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::ofstream ofs(filename);
        if (ofs) {
            ofs.write(data.data(), data.size());
            server->SendResponse(client_addr, request_id, filename);
        }
        else {
            server->SendResponse(client_addr, request_id, "save failed");
        }
    });

    /*
     * 4. 启动服务器
     */
    printf("Receiver started. SocketFile=%s\n", socket_file);
    g_server->Start();

    return 0;
}
