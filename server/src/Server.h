#pragma once

#include "http/HttpServer.h"
#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>

class Server: public QObject {
        Q_OBJECT

    public:
        explicit Server(QObject *parent = nullptr);
        ~Server() override;

        // 初始化服务器 - 加载配置并设置组件
        bool initialize(const QString &configPath = "config/server.json");

        // 启动服务器
        bool start();

        // 停止服务器
        void stop();

        // 获取服务器状态
        bool isRunning() const;

    private slots:
        // HTTP请求处理
        void handleHttpRequest(const HttpRequest &request);

        // 检查数据库连接池状态
        void checkDatabasePools();

    private:
        // 注册API路由
        void registerRoutes();

        // 初始化组件
        bool initializeConfig(const QString &configPath);
        bool initializeDatabase();
        bool initializeApiManager();
        bool initializeHttpServer();

        // 服务器组件
        HttpServer m_httpServer;

        // 状态和配置
        bool        m_initialized;
        bool        m_running;
        QVariantMap m_serverConfig;

        // 状态检查定时器
        QTimer m_healthCheckTimer;
};
