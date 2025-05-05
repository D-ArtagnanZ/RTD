#include "Server.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <csignal>
#include <iostream>

// 全局服务器指针，用于信号处理
Server *g_server = nullptr;

// 信号处理函数
void signalHandler(int signal)
{
    std::cout << "接收到信号: " << signal << std::endl;
    if (g_server) {
        g_server->stop();
        // 让应用程序退出事件循环
        QCoreApplication::quit();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("RTD+ Server");
    QCoreApplication::setApplicationVersion("1.0.0");

    // 设置信号处理
    signal(SIGINT, signalHandler);     // Ctrl+C
    signal(SIGTERM, signalHandler);    // kill命令

    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("RTD+ 派工服务器");
    parser.addHelpOption();
    parser.addVersionOption();

    // 配置文件选项
    QCommandLineOption configOption(QStringList() << "c" << "config",
                                    "配置文件路径",
                                    "file",
                                    "config/server.json");
    parser.addOption(configOption);

    parser.process(app);

    QString configPath = parser.value(configOption);

    // 创建服务器实例
    Server server;
    g_server = &server;

    // 初始化并启动服务器
    if (!server.initialize(configPath)) {
        std::cerr << "服务器初始化失败\n";
        return 1;
    }

    if (!server.start()) {
        std::cerr << "服务器启动失败\n";
        return 1;
    }

    // 进入事件循环
    return app.exec();
}
