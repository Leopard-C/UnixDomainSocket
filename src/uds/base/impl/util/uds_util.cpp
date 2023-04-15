#include "uds_util.h"
#include <chrono>
#include <thread>

namespace ic {
namespace uds {
namespace util {

/**
 * @brief 最大发送缓冲区，单次发送包的有效数据的最大长度.
 * 
 * @details IP首部(20) + UDP首部(8) + 自定义头部(16) + 65491 = 65535(64KB)
 */
static const size_t PACKET_CUSTOM_HEADER_SIZE = 16;
static const size_t MAX_SEND_PACKET_DATA_SIZE = 65491;
static const size_t MAX_SEND_BUFFER_SIZE = PACKET_CUSTOM_HEADER_SIZE + MAX_SEND_PACKET_DATA_SIZE;

/**
 * @brief 最大接收缓冲区，一般为64KB，这里设的大一些.
 */
static const size_t MAX_RECV_BUFFER_SIZE = 131072; /* 128KB */

/**
 * @brief 发送数据(发送一个数据报).
 * 
 * @details 格式： 8字节(id) + 4字节(packets_total) + 4字节(packet_seq) + body
 * @details id: 请求ID，由客户端保证唯一.
 * @details packets_total: 分包数量.
 * @details packet_seq: 当前分包序列号.
 */
static bool s_send_data(
    int fd,
    const sockaddr_un& target_addr,
    const char* data, size_t len,
    int64_t request_id, uint32_t packets_total, uint32_t packet_seq)
{
    static thread_local char send_buffer[MAX_SEND_BUFFER_SIZE + 1];
    memset(send_buffer,      0,               sizeof(send_buffer));
    memcpy(send_buffer,      &request_id,     8);
    memcpy(send_buffer + 8,  &packets_total,  4);
    memcpy(send_buffer + 12, &packet_seq,     4);
    memcpy(send_buffer + 16, data,          len);
    size_t buffer_len = 16 + len;
    //printf("send %ld\n", request_id);
    ssize_t n = ::sendto(fd, send_buffer, buffer_len, 0, (const sockaddr*)&target_addr, sizeof(sockaddr_un));
    //printf("sent %ld\n", request_id);
    if (n < 0) {
        return false;
    }
    else if (static_cast<size_t>(n) != buffer_len) {
        fprintf(stderr, "sendto() failed, incomplete. %ld/%ld bytes", n, buffer_len);
        return false;
    }
    return true;
}

/**
 * @brief 发送数据，如果数据太长，则进行分包发送.
 */
bool send_data(int fd, const sockaddr_un& target_addr, int64_t request_id, const std::string& data) {
    size_t len = data.length();
    if (len <= MAX_SEND_PACKET_DATA_SIZE) {
        if (!s_send_data(fd, target_addr, data.data(), len, request_id, 1, 1)) {
            return false;
        }
    }
    else {
        uint32_t packets_count = len / MAX_SEND_PACKET_DATA_SIZE, rem = len % MAX_SEND_PACKET_DATA_SIZE;
        if (rem > 0) {
            packets_count += 1;
        }
        uint32_t seq = 1;
        for (size_t i = 0; i < len; i += MAX_SEND_PACKET_DATA_SIZE, ++seq) {
            size_t send_len = (seq < packets_count || rem == 0) ? MAX_SEND_PACKET_DATA_SIZE : rem;
            if (!s_send_data(fd, target_addr, data.data() + i, send_len, request_id, packets_count, seq)) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief 接收数据.
 */
static ssize_t s_recv_data(
    int fd, fd_set* read_fds,
    char* buffer, size_t buffer_size,
    sockaddr_un* from_addr, socklen_t* from_addr_len)
{
    FD_ZERO(read_fds);
    FD_SET(fd, read_fds);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    int ret = ::select(fd + 1, read_fds, NULL, NULL, &timeout);
    if (ret < 0) {
        if (errno == EINTR) {
            // select() interrupted by signal
        }
        else {
            fprintf(stderr, "select() failed. errno=%d, errmsg=%s", errno, strerror(errno));
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return -1;
    }
    else if (ret == 0) { /* 超时 */
        return -1;
    }
    else if (FD_ISSET(fd, read_fds)) {
        memset(buffer, 0, buffer_size);
        ssize_t n = ::recvfrom(fd, buffer, buffer_size, 0, (sockaddr*)from_addr, from_addr_len);
        if (n >= 0) {
            return n;
        }
        else {
            fprintf(stderr, "recvfrom() failed. errno=%d, errmsg=%s", errno, strerror(errno));
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    return -1;
}

/**
 * @brief 接收数据.
 * 
 * @details 格式： 8字节(id) + 4字节(packets_total) + 4字节(packet_seq) + body
 * @details id: 请求ID，由客户端保证唯一，如果分包，则用于组包。响应数据中需要带有该ID.
 * @details packets_total: 分包数量.
 * @details packet_seq: 当前分包序列号.
 */
bool recv_data(int fd, fd_set* read_fds, Packet* packet, sockaddr_un* from_addr, socklen_t* from_addr_len) {
    thread_local char recv_buffer[MAX_RECV_BUFFER_SIZE + 1];
    ssize_t n = s_recv_data(fd, read_fds, recv_buffer, sizeof(recv_buffer), from_addr, from_addr_len);
    if (n < 0) {
        return false;
    }
    else if (n < 16) {
        recv_buffer[n] = '\0';
        fprintf(stderr, "Invalid data. len=%d<16", static_cast<int>(n));
        return false;
    }
    else {
        packet->arrive_time = std::chrono::steady_clock::now();
        packet->id = *((int64_t*)recv_buffer);
        packet->total = *((uint32_t*)(recv_buffer + 8));
        packet->seq = *((uint32_t*)(recv_buffer + 12));
        packet->data.assign(recv_buffer + 16, n - 16);
        return true;
    }
}

} // namespace util
} // namespace uds
} // namespace ic
