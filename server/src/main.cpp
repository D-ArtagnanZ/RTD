#include "api_handler.h"
#include "db_manager.h"
#include "http_server.h"
#include <csignal>
#include <iostream>

// 全局HTTP服务器实例
std::unique_ptr<HttpServer> g_server;

// 信号处理
void signalHandler(int signal)
{
    std::cout << "接收到信号: " << signal << "，正在关闭服务器..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char *argv[])
{
    try {
        // 设置信号处理
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // 初始化数据库管理器
        if (!DbManager::instance().initialize()) {
            std::cerr << "初始化数据库管理器失败" << std::endl;
            return 1;
        }

        // 初始化API处理器
        if (!ApiHandler::instance().initialize()) {
            std::cerr << "初始化API处理器失败" << std::endl;
            return 1;
        }

        // 创建并启动HTTP服务器
        int port = 8080;
        if (argc > 1) {
            port = std::stoi(argv[1]);
        }

        g_server = std::make_unique<HttpServer>(port);
        g_server->start();

        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << "运行时错误: " << e.what() << std::endl;
        return 1;
    }
}
