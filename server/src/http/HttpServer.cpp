#include "http/HttpServer.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

HttpServer::HttpServer(QObject *parent)
    : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &HttpServer::handleNewConnection);

    // 设置默认的404处理
    m_notFoundHandler = [](const HttpRequest &) {
        HttpResponse response(404);
        response.setHeader("Content-Type", "application/json");

        QJsonObject json;
        json["success"] = false;
        json["error"]   = "Not Found";

        response.setBody(QJsonDocument(json).toJson());
        return response;
    };
}

bool HttpServer::start(int port, const QString &address)
{
    return m_server.listen(QHostAddress(address), port);
}

void HttpServer::stop()
{
    if (m_server.isListening()) {
        m_server.close();
    }
}

bool HttpServer::isRunning() const
{
    return m_server.isListening();
}

void HttpServer::registerRoute(const QString &method, const QString &path, const RequestHandler &handler)
{
    Route route;
    route.pathRegex = pathToRegex(path);
    route.handler   = handler;

    m_routes[method].append(route);

    qDebug() << "注册路由:" << method << path;
}

void HttpServer::setNotFoundHandler(const RequestHandler &handler)
{
    m_notFoundHandler = handler;
}

void HttpServer::handleNewConnection()
{
    QTcpSocket *socket = m_server.nextPendingConnection();
    if (!socket) {
        return;
    }

    connect(socket, &QTcpSocket::readyRead, this, &HttpServer::handleReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &HttpServer::handleDisconnected);
}

void HttpServer::handleReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    QByteArray requestData = socket->readAll();

    // 解析HTTP请求
    HttpRequest request(requestData);

    // 发出请求接收信号
    emit requestReceived(request);

    // 处理请求
    HttpResponse response;
    if (!matchRoute(request.method(), request.path(), request, response)) {
        response = m_notFoundHandler(request);
    }

    // 发送响应
    socket->write(response.toByteArray());

    // 如果是HTTP/1.0或明确设置了Connection: close，则关闭连接
    if (request.version() == "HTTP/1.0" || request.headers().value("Connection", "").toLower() == "close") {
        socket->disconnectFromHost();
    }

    // 发出响应就绪信号
    emit responseReady(response, socket);
}

void HttpServer::handleDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

bool HttpServer::matchRoute(const QString &method, const QString &path, const HttpRequest &request, HttpResponse &response)
{
    if (!m_routes.contains(method)) {
        return false;
    }

    const QList<Route> &routes = m_routes[method];
    for (const Route &route: routes) {
        QRegularExpressionMatch match = route.pathRegex.match(path);
        if (match.hasMatch()) {
            // 提取路径参数
            QMap<QString, QString> pathParams = extractPathParams(route.pathRegex, path);

            // 创建新的请求对象，包含路径参数
            HttpRequest requestWithParams = request;
            requestWithParams.setPathParameters(pathParams);

            // 处理请求
            response = route.handler(requestWithParams);
            return true;
        }
    }

    return false;
}

QMap<QString, QString> HttpServer::extractPathParams(const QRegularExpression &regex, const QString &path)
{
    QMap<QString, QString> params;

    QRegularExpressionMatch match = regex.match(path);
    if (match.hasMatch()) {
        QStringList paramNames = regex.namedCaptureGroups();

        for (int i = 1; i < paramNames.size(); ++i) {
            QString name = paramNames.at(i);
            if (!name.isEmpty()) {
                params[name] = match.captured(i);
            }
        }
    }

    return params;
}

QRegularExpression HttpServer::pathToRegex(const QString &path)
{
    // 将路径模式转换为正则表达式
    // 例如 /api/users/:id 转换为 ^/api/users/(?<id>[^/]+)$
    QString pattern = path;

    // 转义所有正则表达式特殊字符，但保留冒号
    pattern.replace(".", "\\.");
    pattern.replace("^", "\\^");
    pattern.replace("$", "\\$");
    pattern.replace("*", "\\*");
    pattern.replace("+", "\\+");
    pattern.replace("?", "\\?");
    pattern.replace("(", "\\(");
    pattern.replace(")", "\\)");
    pattern.replace("[", "\\[");
    pattern.replace("]", "\\]");
    pattern.replace("{", "\\{");
    pattern.replace("}", "\\}");
    pattern.replace("|", "\\|");
    pattern.replace("\\", "\\\\");

    // 替换参数占位符，例如:id
    static const QRegularExpression paramRegex(":([a-zA-Z0-9_]+)");
    pattern.replace(paramRegex, "(?<\\1>[^/]+)");

    // 添加开始和结束标记
    pattern = "^" + pattern + "$";

    return QRegularExpression(pattern);
}
