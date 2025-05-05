#include "http/HttpResponse.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

HttpResponse::HttpResponse(int statusCode)
    : m_statusCode(statusCode)
{
    // 设置默认头部
    m_headers["Content-Type"] = "text/html; charset=utf-8";
    m_headers["Connection"] = "keep-alive";
}

void HttpResponse::setStatusCode(int code)
{
    m_statusCode = code;
}

void HttpResponse::setHeader(const QString& name, const QString& value)
{
    m_headers[name] = value;
}

void HttpResponse::setBody(const QByteArray& body)
{
    m_body = body;
}

int HttpResponse::statusCode() const
{
    return m_statusCode;
}

QByteArray HttpResponse::toByteArray() const
{
    QByteArray response;
    
    // 状态行
    response.append(QString("HTTP/1.1 %1 %2\r\n").arg(m_statusCode).arg(getStatusText(m_statusCode)).toUtf8());
    
    // 头部
    for (auto it = m_headers.constBegin(); it != m_headers.constEnd(); ++it) {
        response.append(QString("%1: %2\r\n").arg(it.key(), it.value()).toUtf8());
    }
    
    // 设置Content-Length
    response.append(QString("Content-Length: %1\r\n").arg(m_body.length()).toUtf8());
    
    // 头部和体的分隔符
    response.append("\r\n");
    
    // 响应体
    response.append(m_body);
    
    return response;
}

HttpResponse HttpResponse::fromJson(const QByteArray& json, int statusCode)
{
    HttpResponse response(statusCode);
    response.setHeader("Content-Type", "application/json");
    response.setBody(json);
    return response;
}

HttpResponse HttpResponse::fromMap(const QMap<QString, QVariant>& map, int statusCode)
{
    QJsonObject json = QJsonObject::fromVariantMap(QVariantMap(map));
    return fromJson(QJsonDocument(json).toJson(), statusCode);
}

QString HttpResponse::getStatusText(int code)
{
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}
