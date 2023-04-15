#include <chrono>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include "uds/json/server.h"
#include "uds/json/router.h"
#include "uds/json/request.h"
#include "uds/json/response.h"

std::shared_ptr<ic::uds::Server> g_server;
const char* socket_file = "/dev/shm/.simple_server.sock";
const size_t thread_pool_size = 8;

void CatchCtrlC(int sig) {
    if (g_server) {
        g_server->Stop();
    }
}

std::string current_time() {
    return std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

/***************************************************
 *
 * 统一返回如下格式：
 *   {
 *     "code": 0,
 *     "msg": "OK",
 *     "data": xxx
 *   }
 * 
***************************************************/
void ResponseResult(ic::uds::Response& res, int code, const std::string& msg, const Json::Value& data = Json::nullValue) {
    res["code"] = code;
    res["msg"] = msg;
    if (data) {
        res["data"] = data;
    }
}

void ResponseOk(ic::uds::Response& res, const Json::Value& data = Json::nullValue){
    ResponseResult(res, 0, "OK", data);
}

void ResponseInvalidParam(ic::uds::Response& res){
    ResponseResult(res, -1, "Invalid param");
}

void ResponseInternalServerError(ic::uds::Response& res){
    ResponseResult(res, -2, "Internal server error");
}

int main() {
    signal(SIGINT, CatchCtrlC);

    /*
     * 1. 创建服务器
     */
    g_server = std::make_shared<ic::uds::Server>();

    /*
     * 2. 初始化服务器
     */
    std::error_code ec;
    g_server->Init(socket_file, thread_pool_size, ec);
    if (ec) {
        printf("[Error] UDS.Server init failed. %s\n", ec.message().c_str());
        return 1;
    }

    /*
     * 3. 配置路由
     */
    ic::uds::Router* router = g_server->router();
    router->AddRoute("/hello", [](ic::uds::Request& req, ic::uds::Response& res){
        ResponseOk(res, "hello");
    });
    router->AddRoute("/echo", [](ic::uds::Request& req, ic::uds::Response& res){
        if (!req["text"].isString()) {
            return ResponseInvalidParam(res);
        }
        res["data"]["text"] = req["text"];
        return ResponseOk(res);
    });
    router->AddRoute("/circle/area", "计算指定半径的圆面积", [](ic::uds::Request& req, ic::uds::Response& res){
        if (!req["radius"].isDouble()) {
            return ResponseInvalidParam(res);
        }
        double radius = req["radius"].asDouble();
        const double PI = 3.141592653589793;
        res["data"]["area"] = PI * radius * radius;
        return ResponseOk(res);
    });
    router->AddRoute("/image/upload", "上传图片", [](ic::uds::Request& req, ic::uds::Response& res){
        if (!req["filename"].isString() || !req["ext"].isString()) {
            return ResponseInvalidParam(res);
        }
        auto filename = req["filename"].asString();
        auto ext = req["ext"].asString();
        if (filename.empty() || ext.empty()) {
            return ResponseInvalidParam(res);
        }
        if (ext[0] != '.') {
            ext = "." + ext;
        }
        auto& image = req.GetBody("image"); /* 图片文件数据 */
        if (image.empty()) {
            return ResponseInvalidParam(res);
        }
        filename = "/dev/shm/" + current_time() + "-" + filename + ext;
        std::ofstream ofs(filename);
        if (!ofs) {
            return ResponseInternalServerError(res);
        }
        ofs.write(image.data(), image.size());
        return ResponseOk(res);
    });
    router->AddRoute("/server/stop", "关闭服务器", [](ic::uds::Request& req, ic::uds::Response& res){
        req.svr()->Stop();
        return ResponseOk(res);
    });

    /*
     * 4. 启动服务器
     */
    printf("Server started. SocketFile=%s\n", socket_file);
    g_server->Start();

    return 0;
}
