/**
 * @file packet.h
 * @brief UDS数据包.
 * @author Leopard-C (leopard.c@outlook.com)
 * @version 0.1
 * @date 2023-03-11
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_UDS_BASE_IMPL_PACKET_H_
#define IC_UDS_BASE_IMPL_PACKET_H_
#include <chrono>
#include <string>
#include <vector>

namespace ic {
namespace uds {

/**
 * @brief 数据包.
 */
struct Packet {
    uint32_t total;  /* 数据包总量 */
    uint32_t seq;    /* 当前数据包的序列号 */
    int64_t id;      /* 完整数据包的ID */
    std::chrono::steady_clock::time_point arrive_time;  /* 当前数据包到达时间 */
    std::string data;  /* 数据包内容 */
};

/**
 * @brief 数据包数组(各个分包组成的完整数据包).
 */
struct Packets : std::vector<Packet*> {
    ~Packets();
    /**
     * @brief 组包，只能调用1次，调用后所有分包被清空.
     */
    std::string Merge();
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_BASE_IMPL_PACKET_H_
