#ifndef IC_UDS_JSON_MESSAGE_H_
#define IC_UDS_JSON_MESSAGE_H_
#include <string>
#include <jsoncpp/json/value.h>

namespace ic {
namespace uds {

class Message {
public:
    Message(int64_t id = -1) : id_(id) {}

public:
    /**
     * @brief 当前请求ID.
     */
    int64_t id() const { return id_; }

    /**
     * @brief 返回json_[":param"]
     */
    const Json::Value& param() const { return json_[":param"]; }

    /**
     * @brief 重载[]运算符，返回json_[":param"][name]的引用.
     */
    Json::Value& operator[](const char* name) { return json_[":param"][name]; }
    Json::Value& operator[](const std::string& name) { return json_[":param"][name]; }

    std::map<std::string, std::string>& body() { return body_; }
    const std::string& GetBody(const std::string& name) const;
    void AddBody(const std::string& name, const std::string& value) { body_[name] = value; }

protected:
    void set_id(int64_t id) { id_ = id; }

    /**
     * @brief 序列化为字符串，用于发送.
     */
    std::string Serialize(bool clear_body = false);

    /**
     * @brief 反序列化，解析字符串数据.
     */
    bool Deserialize(const std::string& data);

private:
    bool ParseJson(const std::string& data, size_t start, size_t end);
    bool ParseBody(const std::string& data, size_t body_start);

protected:
    /**
     * @brief 当前请求对应的ID.
     */
    int64_t id_{-1};

    /**
     * @brief JSON数据
     * 
     * @details 示例如下
     *  {
     *    ":path": "/user/add",
     *    ":param": {
     *       "uid": 1,
     *       "name": "ic",
     *       "role": "admin"
     *    },
     *    ":body": [
     *       {
     *          ":name": "image",
     *          ":offset": 756,
     *          ":length": 4096
     *       }
     *    ]
     *  }
     */
    Json::Value json_;

    /**
     * @brief 非JSON数据.
     * 
     * @details 元数据信息存在 json_[":body"]中.
     * @details 可以存储任何数据，如大文本数据、二进制数据等.
     */
    std::map<std::string, std::string> body_;
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_JSON_MESSAGE_H_
