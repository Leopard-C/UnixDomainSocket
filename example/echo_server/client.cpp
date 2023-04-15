#include <stdio.h>
#include "uds/base/base_client.h"

int main() {
    const char* server_socket_file = "/dev/shm/.echo_server.sock";
    const char* client_socket_file = "/dev/shm/.echo_client.sock";

    /*
     * 1. 创建客户端
     */
    ic::uds::BaseClient client;

    /*
     * 2. 初始化客户端
     */
    std::error_code ec;
    client.Init(server_socket_file, client_socket_file, ec);
    if (ec) {
        printf("[Error] UDS.BaseClient init failed. %s\n", ec.message().c_str());
        return 1;
    }

    for (int i = 1; i <= 100; ++i) {
        /*
         * 3. 发送请求，并获取返回内容
         */
        char content[100] = { 0 };
        sprintf(content, "Hello world! [%d]", i);
        std::error_code ec;
        std::string response;
        client.SendRequest(content, &response, 2000, ec);
        if (ec) {
            printf("SendRequest() failed. %s\n\n", ec.message().c_str());
        }
        else {
            printf("sent: %s\nrecv: %s\n\n", content, response.c_str());
        }
    }

    return 0;
}
