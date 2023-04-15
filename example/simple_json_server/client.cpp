#include <chrono>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/json.h>
#include "uds/json/client.h"

std::string current_time() {
    return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
}

std::string read_image(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        printf("Read image '%s' failed\n", filename.c_str());
        return {};
    }
    return {std::istreambuf_iterator<char>(ifs.rdbuf()), std::istreambuf_iterator<char>()};
}

int main() {
    std::string server_socket_file = "/dev/shm/.simple_server.sock";
    std::string client_socket_file = "/dev/shm/.simple_client_" + current_time() + ".sock";

    ic::uds::Client client;
    std::error_code ec;
    client.Init(server_socket_file, client_socket_file, ec);
    if (ec) {
        std::cout << "[Error] UDS.Client init failed. " << ec.message() << std::endl;
        return 1;
    }

    Json::FastWriter fw;
    fw.omitEndingLineFeed();
    fw.emitUTF8();

    {
        ic::uds::Request request("/hello");
        auto response = client.SendRequest(request, 3000);
        std::cout << "Request: /echo&text='Hello World!'" << std::endl;
        std::cout << "Response.status:  " << (int)response.status() << std::endl;
        std::cout << "Response.message: " << response.message() << std::endl;
        std::cout << "Response.content: " << fw.write(response.data()) << std::endl << std::endl;
    }

    {
        ic::uds::Request request("/echo");
        request["text"] = "Hello World!";
        auto response = client.SendRequest(request, 3000);
        std::cout << "Request: /echo&text='Hello World!'" << std::endl;
        std::cout << "Response.status:  " << (int)response.status() << std::endl;
        std::cout << "Response.message: " << response.message() << std::endl;
        std::cout << "Response.content: " << fw.write(response.data()) << std::endl << std::endl;
    }

    /* 上传图片 */
    {
        ic::uds::Request request("/circle/area");
        request["radius"] = 2.6;
        auto response = client.SendRequest(request, 3000);
        std::cout << "Request: /circle/area?radius=2.6" << std::endl;
        std::cout << "Response.status:  " << (int)response.status() << std::endl;
        std::cout << "Response.message: " << response.message() << std::endl;
        std::cout << "Response.content: " << fw.write(response.data()) << std::endl << std::endl;
    }

    /* 计算圆面积 */
    {
        ic::uds::Request request("/image/upload");
        request["filename"] = "debian-logo";
        request["ext"] = ".png";
        request.AddBody("image", read_image("/usr/share/pixmaps/debian-logo.png"));
        auto response = client.SendRequest(request, 3000);
        std::cout << "Request: /image/upload?filename=debian-logo&ext=.png" << std::endl;
        std::cout << "Response.status:  " << (int)response.status() << std::endl;
        std::cout << "Response.message: " << response.message() << std::endl;
        std::cout << "Response.content: " << fw.write(response.data()) << std::endl << std::endl;
    }

    /* 不存在的路径 */
    {
        ic::uds::Request request("/an/invalid/path");
        auto response = client.SendRequest(request, 3000);
        std::cout << "Request: /an/invalid/path" << std::endl;
        std::cout << "Response.status:  " << (int)response.status() << std::endl;
        std::cout << "Response.message: " << response.message() << std::endl;
        std::cout << "Response.content: " << fw.write(response.data()) << std::endl << std::endl;
    }

    /* 关闭服务器(退出当前程序) */
    {
        ic::uds::Request request("/server/stop");
        auto response = client.SendRequest(request, 3000);
        std::cout << "Request: /server/stop" << std::endl;
        std::cout << "Response.status:  " << (int)response.status() << std::endl;
        std::cout << "Response.message: " << response.message() << std::endl;
        std::cout << "Response.content: " << fw.write(response.data()) << std::endl << std::endl;
    }

    return 0;
}
