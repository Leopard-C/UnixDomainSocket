#include <chrono>
#include <memory>
#include <thread>
#include <vector>
#include <stdio.h>
#include "uds/base/base_client.h"

const char* server_socket_file = "/dev/shm/.benchmark_server.sock";
const size_t kThreadsCount = 6;
const size_t kDataLength = 8192;   /* 4KB */
const size_t kSendTimes = 10000;
bool has_error = false;
std::shared_ptr<ic::uds::BaseClient> g_client;

std::string current_time() {
    return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
}

std::string get_temp_client_socket_filename() {
    return "/dev/shm/.benchmark_client_" + current_time() + "_"
        + std::to_string(rand()) + std::to_string(rand()) + ".sock";
}

void thread_function(int tid) {
    std::string text(kDataLength, 'X');
    for (size_t i = 0; i < kSendTimes; ++i) {
        std::error_code ec;
        std::string response;
        int64_t id = g_client->SendRequest(text, &response, 1000, ec);
        //printf("[%lu] [%ld]\n", i + 1, id);
        if (ec) {
            has_error = true;
            printf("[%d] [%ld] SendRequest() failed. %s\n", tid, id, ec.message().c_str());
        }
    }
}

void run() {
    std::string client_socket_file = get_temp_client_socket_filename();
    printf("%s\n", client_socket_file.c_str());

    g_client = std::make_shared<ic::uds::BaseClient>();
    std::error_code ec;
    g_client->Init(server_socket_file, client_socket_file, ec);
    if (ec) {
        printf("[Error] UDS.BaseClient init failed. %s\n", ec.message().c_str());
        return;
    }

    std::vector<std::thread> vec;
    vec.reserve(kThreadsCount);
    for (size_t i = 0; i < kThreadsCount; ++i) {
        vec.emplace_back(thread_function, i);
    }
    for (size_t i = 0; i < kThreadsCount; ++i) {
        vec[i].join();
    }
}

int main() {
    auto time_start = std::chrono::steady_clock::now();
    run();
    auto time_end = std::chrono::steady_clock::now();
    size_t time_total_us = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start).count();
    size_t rps = double(kThreadsCount * kSendTimes * 1000000LL) / time_total_us;
    if (!has_error) {
        printf("%lu r/s\n", rps);
    }
    return 0;
}
