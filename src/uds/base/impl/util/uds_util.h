/**
 * @file uds_util.h
 * @brief UDS辅助函数.
 * @author Leopard-C (leopard.c@outlook.com)
 * @version 0.1
 * @date 2023-03-11
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_UDS_BASE_IMPL_UTIL_H_
#define IC_UDS_BASE_IMPL_UTIL_H_
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include "../uds_packet.h"

namespace ic {
namespace uds {
namespace util {

/**
 * @brief 发送数据.
 */
bool send_data(int fd, const sockaddr_un& target_addr, int64_t request_id, const std::string& data);

/**
 * @brief 接收数据.
 */
bool recv_data(int fd, fd_set* read_fds, Packet* packet, sockaddr_un* from_addr, socklen_t* from_addr_len);

} // namespace util
} // namespace uds
} // namespace ic

#endif // IC_UDS_BASE_IMPL_UTIL_H_
