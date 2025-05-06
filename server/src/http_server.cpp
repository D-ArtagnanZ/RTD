#include "http_server.h"
#include "api_handler.h"
#include <iostream>

HttpServer::HttpServer(int port)
    : m_port(port)
{
    setupRoutes();
}

void HttpServer::start()
{
    std::cout << "HTTP服务器启动，监听端口: " << m_port << std::endl;
    m_app.port(m_port).multithreaded().run();
}

void HttpServer::stop()
{
    m_app.stop();
}

void HttpServer::setupRoutes()
{
    // 健康检查端点
    CROW_ROUTE(m_app, "/health")
    ([]() {
        return crow::response(200, "{\"status\":\"ok\"}");
    });

    // API端点
    CROW_ROUTE(m_app, "/api/<string>")
    ([this](const crow::request &req, std::string queryId) {
        return handleApiRequest(req, queryId);
    });

    // 处理404
    CROW_CATCHALL_ROUTE(m_app)
    ([](crow::response &res) {
        res.code = 404;
        res.body = "{\"error\":\"找不到请求的资源\"}";
        res.set_header("Content-Type", "application/json");
    });
}

crow::response HttpServer::handleApiRequest(const crow::request &req, const std::string &queryId)
{
    // 检查Content-Type
    if (req.get_header_value("Content-Type") != "application/json") {
        return crow::response(400, "{\"error\":\"请求必须是JSON格式\"}");
    }

    try {
        // 解析请求参数
        json params = json::parse(req.body);

        // 处理API请求
        json result = ApiHandler::instance().handleRequest(queryId, params);

        // 构建响应
        return crow::response(200, result.dump());
    }
    catch (const std::exception &e) {
        return crow::response(400, "{\"error\":\"" + std::string(e.what()) + "\"}");
    }
}
