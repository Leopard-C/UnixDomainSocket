#include <chrono>
#include <stdio.h>
#include "uds/base/base_client.h"

void PrintUsage() {
    printf("Usage:\n");
    printf("  uds-base-cli {ServerSocketFile} {Data} {RecvResponse=true|false} [{timeout=3000}]\n\n");
    printf("Example:\n");
    printf("  uds-base-cli \"/dev/shm/server.sock\" \"Hello World!\" true 3000\n");
}

std::string current_time() {
    return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
}

int main(int argc, char* argv[]) {
    std::string client_socket_file = "/dev/shm/.uds_base_cli_" + current_time() + ".sock";

    if (argc < 4 || argc > 5) {
        PrintUsage();
        return 1;
    }

    std::string server_socket_file(argv[1]);
    std::string content(argv[2]);
    bool recv_response = false;
    if (strcmp(argv[3], "true") == 0) {
        recv_response = true;
    }
    else if (strcmp(argv[3], "false") == 0) {
        recv_response = false;
    }
    else {
        printf("Invalid param: RecvResponse=%s\n", argv[3]);
        PrintUsage();
        return 1;
    }

    uint32_t timeout = 3000;
    if (argc == 5) {
        timeout = std::atoi(argv[4]);
    }

    ic::uds::BaseClient client;
    std::error_code ec;
    client.Init(server_socket_file, client_socket_file, ec);
    if (ec) {
        printf("[Error] UDS.BaseClient init failed. %s\n", ec.message().c_str());
        return 2;
    }

    if (recv_response) {
        std::string response;
        client.SendRequest(content, &response, timeout, ec);
        if (ec) {
            printf("SendRequest() failed. %s\n\n", ec.message().c_str());
            return 3;
        }
        printf(" Request: (%lu bytes) %s\n", content.size(), content.c_str());
        printf("Response: (%lu bytes) %s\n", response.size(), response.c_str());
        return 0;
    }
    else {
        client.Send(content, ec);
        if (ec) {
            printf("Send() failed. %s\n\n", ec.message().c_str());
        }
        printf("Sent: (%lu bytes) %s\n", content.size(), content.c_str());
        return 0;
    }
}
