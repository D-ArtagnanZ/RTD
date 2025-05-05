#pragma once

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QTcpServer>
#include <QTcpSocket>
#include <functional>

class HttpServer: public QObject {
        Q_OBJECT

    public:
        using RequestHandler = std::function<HttpResponse(const HttpRequest &)>;

        explicit HttpServer(QObject *parent = nullptr);

        // 启动服务器
        bool start(int port = 8080, const QString &address = "0.0.0.0");

        // 停止服务器
        void stop();

        // 注册路由
        void registerRoute(const QString &method, const QString &path, const RequestHandler &handler);

        // 设置错误处理
        void setNotFoundHandler(const RequestHandler &handler);

        // 检查服务器是否运行
        bool isRunning() const;

    signals:
        void requestReceived(const HttpRequest &request);
        void responseReady(const HttpResponse &response, QTcpSocket *socket);

    private slots:
        void handleNewConnection();
        void handleReadyRead();
        void handleDisconnected();

    private:
        struct Route {
                QRegularExpression pathRegex;
                RequestHandler     handler;
        };

        QTcpServer                  m_server;
        QMap<QString, QList<Route>> m_routes;    // method -> [routes]
        RequestHandler              m_notFoundHandler;

        // 路径匹配
        bool matchRoute(const QString &method, const QString &path, const HttpRequest &request, HttpResponse &response);

        // 提取路径参数
        QMap<QString, QString> extractPathParams(const QRegularExpression &regex, const QString &path);

        // 路径转换为正则表达式
        QRegularExpression pathToRegex(const QString &path);
};
