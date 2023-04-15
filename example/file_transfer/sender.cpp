#include <fstream>
#include "uds/base/base_client.h"

const char* server_socket_file = "/dev/shm/.file_receiver.sock";
const char* client_socket_file = "/dev/shm/.file_sender.sock";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage:\n  sender {filepath}\n");
        return 1;
    }

    /* 读取文件内容 */
    std::string filename(argv[1]);
    std::ifstream ifs(filename, std::ios::in | std::ios::binary);
    if (!ifs) {
        printf("[Error] Open local file failed\n");
        return 2;
    }
    std::string content(std::istreambuf_iterator<char>(ifs.rdbuf()), std::istreambuf_iterator<char>());
    ifs.close();

    /* 创建并初始化客户端 */
    ic::uds::BaseClient client;
    std::error_code ec;
    client.Init(server_socket_file, client_socket_file, ec);
    if (ec) {
        printf("[Error] UDS.BaseClient init failed. %s\n", ec.message().c_str());
        return 1;
    }

    /* 发送文件内容 */
    std::string response;
    client.SendRequest(content, &response, 2000, ec);
    if (ec) {
        printf("[Error] SendRequest() failed. %s\n", ec.message().c_str());
        return 2;
    }

    printf("Send file: %s\n", filename.c_str());
    printf(" Response: (%d bytes) %s\n", (int)response.size(), response.c_str());

    return 0;
}
