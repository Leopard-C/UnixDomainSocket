#ifndef IC_UDS_JSON_RESPONSE_
#define IC_UDS_JSON_RESPONSE_
#include "message.h"

namespace ic {
namespace uds {

class Server;
class Client;
class Router;

class Response : public Message {
public:
    friend class Server;
    friend class Client;
    friend class Router;

    enum class Status {
        Success = 0,
        BadRequest = 1,
        BadResponse,
        InvalidPath,
        NotInitialized,
        SendFailed,
        RecvFailed,
        Timeout,
        UnknownError
    };

    Response(int64_t id = -1) : Message(id) {}

public:
    bool success() const { return status_ == Status::Success; }
    Status status() const { return status_; }
    const char* message() const;
    const Json::Value& data() const { return param(); }

protected:
    /**
     * @brief 设置状态码.
     */
    void set_status(Status status) { status_ = status; }

    /**
     * @brief 序列化为字符串，用于发送.
     */
    std::string Serialize(bool clear_body = false);

    /**
     * @brief 解析接受到的请求.
     */
    bool Deserialize(const std::string& data);

private:
    Status status_ = Status::BadRequest;
};

} // namespace uds
} // namespace ic

#endif // IC_UDS_JSON_RESPONSE_
