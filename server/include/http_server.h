#pragma once

#include <crow.h>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class HttpServer {
    public:
        HttpServer(int port = 8080);

        // 启动服务器
        void start();

        // 停止服务器
        void stop();

    private:
        // 注册API路由
        void setupRoutes();

        // 处理API请求
        crow::response handleApiRequest(const crow::request &req, const std::string &queryId);

        int             m_port;
        crow::SimpleApp m_app;
};
