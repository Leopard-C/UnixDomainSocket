#include <memory>
#include <stdio.h>
#include <signal.h>
#include "uds/base/base_server.h"

std::shared_ptr<ic::uds::BaseServer> g_server;
const char* socket_file = "/dev/shm/.benchmark_server.sock";
const size_t thread_pool_size = 8;

/* 捕获Ctrl+C事件 */
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
     * 2. 初始化服务器
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
        //printf(
        //    "client: %s\nrequest_id: %ld\ncontent: %s\n\n",
        //    client_addr.sun_path, request_id, data.c_str()
        //);
        server->SendResponse(client_addr, request_id, data);
    });

    /*
     * 4. 启动服务器
     */
    printf("Server started. SocketFile=%s\n", socket_file);
    g_server->Start();

    return 0;
}
