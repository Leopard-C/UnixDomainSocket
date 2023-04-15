#include <chrono>
#include <jsoncpp/json/json.h>
#include "uds/json/client.h"

void PrintUsage() {
    printf("Usage:\n");
    printf("  uds-cli ${ServerSocketFile} ${route} ${param} [${timeout=3000}]\n\n");
    printf("Example:\n");
    printf("  uds-cli /dev/shm/server.sock /hello \"{\\\"name\\\":\\\"Jack\\\"}\" true 3000\n");
}

std::string current_time() {
    return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
}

int main(int argc, char* argv[]) {
    std::string client_socket_file = "/dev/shm/.uds_json_cli_" + current_time() + ".sock";

    if (argc < 4 || argc > 5) {
        PrintUsage();
        return 1;
    }

    std::string server_socket_file(argv[1]);
    std::string route(argv[2]);
    std::string param(argv[3]);
    uint32_t timeout = 3000;
    if (argc == 5) {
        timeout = std::atoi(argv[4]);
    }

    ic::uds::Client client;
    std::error_code ec;
    client.Init(server_socket_file, client_socket_file, ec);
    if (ec) {
        printf("UDS server init failed\n");
        return 2;
    }

    ic::uds::Request request;
    request.set_path(route);
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(param, root, false) || !root.isObject()) {
        printf("Invalid json param\n");
        return 3;
    }
    for (const auto& mem : root.getMemberNames()) {
        request[mem] = root[mem];
    }

    auto response = client.SendRequest(request, 3000);
    Json::FastWriter fw;
    fw.emitUTF8();
    fw.omitEndingLineFeed();
    printf(
        "Response:\n"
        "  status: %d\n"
        " message: %s\n"
        " content: %s\n",
        (int)response.status(), response.message(), fw.write(response.data()).c_str()
    );

    return 0;
}
