#include "uds_packet.h"
#include <algorithm>

namespace ic {
namespace uds {

Packets::~Packets() {
    for (Packet* packet : *this) {
        if (packet) {
            delete packet;
        }
    }
}

/**
 * @brief 组包，只能调用1次，调用后所有分包被清空.
 */
std::string Packets::Merge() {
    if (this->empty()) {
        return {};
    }

    /* 排序 */
    std::sort(this->begin(), this->end(), [](const Packet* p1, const Packet* p2){
        return p1->seq < p2->seq;
    });

    /* 总数据量大小 */
    size_t total_size = 0;
    for (const auto& packet : *this) {
        total_size += packet->data.size();
    }

    /* 合并 */
    std::string result;
    result.reserve(total_size);
    for (auto& packet : *this) {
        result.append(packet->data);
        delete packet;
        packet = nullptr;
    }
    this->clear();
    return result;
}

} // namespace uds
} // namespace ic
