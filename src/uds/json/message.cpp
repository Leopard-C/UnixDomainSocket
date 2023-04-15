#include "message.h"
#include <jsoncpp/json/json.h>

namespace ic {
namespace uds {

static std::string s_empty_string;

/**
 * @brief 序列化为字符串，用于发送.
 * 
 * @details 格式：json串长度(4字节) + json数据(前面指定的长度) + 非json数据(剩余长度).
 */
std::string Message::Serialize(bool clear_body/* = false*/) {
    size_t body_total_length = 0;
    if (body_.size()) {
        Json::Value& body_info = json_[":body"];
        for (auto iter = body_.begin(); iter != body_.end(); ++iter) {
            Json::Value node;
            node[":name"] = iter->first;
            node[":offset"] = body_total_length;
            node[":length"] = iter->second.length();
            body_info.append(node);
            body_total_length += iter->second.length();
        }
    }
    Json::FastWriter fw;
    fw.emitUTF8();
    fw.omitEndingLineFeed();
    fw.dropNullKeyValues();
    std::string json_string = fw.write(json_);
    unsigned int json_string_length = static_cast<unsigned int>(json_string.length());  /* 必须用unsigned int, 不能用size_t */

    size_t result_length = 4 + json_string_length + body_total_length;
    std::string result(result_length, '\0');
    char* ptr = &(result[0]);
    memcpy(ptr, &json_string_length, 4);
    ptr += 4;
    memcpy(ptr, json_string.c_str(), json_string_length);
    ptr += json_string_length;
    for (auto iter = body_.begin(); iter != body_.end();/* ++iter*/) {
        memcpy(ptr, iter->second.c_str(), iter->second.length());
        ptr += iter->second.length();
        if (clear_body) {
            iter = body_.erase(iter);
        }
        else {
            ++iter;
        }
    }

    return result;
}

/**
 * @brief 反序列化，解析字符串数据.
 */
bool Message::Deserialize(const std::string& data) {
    size_t len = data.length();
    if (len < 16) {
        return false;
    }

    size_t json_len = static_cast<size_t>(*(unsigned int*)(data.c_str()));
    if (json_len > 100000000 || json_len + 4 > len) {
        return false;
    }
    if (!ParseJson(data, 4, 4 + json_len)) {
        return false;
    }
    if (!ParseBody(data, 4 + json_len)) {
        return false;
    }

    return true;
}

/**
 * @brief 获取body数据.
 */
const std::string& Message::GetBody(const std::string& name) const {
    auto iter = body_.find(name);
    if (iter != body_.end()) {
        return iter->second;
    }
    return s_empty_string;
}

/**
 * @brief 解析JSON参数.
 */
bool Message::ParseJson(const std::string& data, size_t start, size_t end) {
    Json::Reader reader;
    const char* doc_start = data.c_str() + start;
    const char* doc_end = data.c_str() + end;
    if (!reader.parse(doc_start, doc_end, json_, false)) {
        return false;
    }
    return json_.isObject() && (!json_[":param"] || json_[":param"].isObject());
}

/**
 * @brief 解析非JSON数据体.
 */
bool Message::ParseBody(const std::string& data, size_t body_start) {
    Json::Value& body_info = json_[":body"];
    if (body_info.isNull()) {
        return true;
    }
    if (!body_info.isArray()) {
        return false;
    }
    size_t len = data.length();
    for (unsigned int i = 0, count = body_info.size(); i < count; ++i) {
        Json::Value& node = body_info[i];
        if (node.isNull() || !node[":name"].isString() || !node[":offset"].isUInt() || !node[":length"].isUInt()) {
            return false;
        }
        std::string name = node[":name"].asString();
        size_t offset = node[":offset"].asUInt();
        size_t length = node[":length"].asUInt();
        size_t start = body_start + offset;
        size_t end = start + length;
        if (name.empty() || start < body_start || end > len) {
            return false;
        }
        body_[name] = data.substr(start, length);
    }
    return true;
}

} // namespace uds
} // namespace ic
