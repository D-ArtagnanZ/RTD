#include "http/HttpRequest.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

HttpRequest::HttpRequest()
{
}

HttpRequest::HttpRequest(const QByteArray &requestData)
{
    parseRequest(requestData);
}

QString HttpRequest::method() const
{
    return m_method;
}

QString HttpRequest::path() const
{
    return m_path;
}

QString HttpRequest::version() const
{
    return m_version;
}

QMap<QString, QString> HttpRequest::headers() const
{
    return m_headers;
}

QByteArray HttpRequest::body() const
{
    return m_body;
}

QString HttpRequest::getQueryParameter(const QString &name) const
{
    return m_query.queryItemValue(name);
}

QString HttpRequest::getPathParameter(const QString &name) const
{
    return m_pathParams.value(name);
}

QString HttpRequest::getBodyParameter(const QString &name) const
{
    // 根据Content-Type解析请求体
    QString contentType = m_headers.value("Content-Type").toLower();

    if (contentType.contains("application/x-www-form-urlencoded")) {
        QUrlQuery query(QString::fromUtf8(m_body));
        return query.queryItemValue(name);
    }
    else if (contentType.contains("application/json")) {
        QJsonDocument doc = QJsonDocument::fromJson(m_body);
        if (doc.isObject()) {
            return doc.object().value(name).toVariant().toString();
        }
    }

    return QString();
}

void HttpRequest::setPathParameters(const QMap<QString, QString> &params)
{
    m_pathParams = params;
}

void HttpRequest::parseRequest(const QByteArray &data)
{
    if (data.isEmpty()) {
        return;
    }

    // 将原始数据分割成请求行、请求头和请求体
    int headerEndPos = data.indexOf("\r\n\r\n");
    if (headerEndPos == -1) {
        qWarning() << "Invalid HTTP request: missing header end";
        return;
    }

    QByteArray headerData = data.left(headerEndPos);
    m_body                = data.mid(headerEndPos + 4);    // 4是"\r\n\r\n"的长度

    // 解析请求头 - 手动分割CRLF行
    QList<QByteArray> headerLines;
    int               pos = 0;
    int               nextLinePos;

    while ((nextLinePos = headerData.indexOf("\r\n", pos)) != -1) {
        headerLines.append(headerData.mid(pos, nextLinePos - pos));
        pos = nextLinePos + 2;    // 跳过"\r\n"
    }

    // 如果有最后一行数据没有被处理
    if (pos < headerData.size()) {
        headerLines.append(headerData.mid(pos));
    }

    if (headerLines.isEmpty()) {
        qWarning() << "Invalid HTTP request: no header lines";
        return;
    }

    // 解析请求行
    QString     requestLine = QString::fromUtf8(headerLines.first());
    QStringList parts       = requestLine.split(' ');

    if (parts.size() >= 3) {
        m_method = parts[0];

        // 解析URL和查询字符串
        QUrl url(parts[1]);
        m_path  = url.path();
        m_query = QUrlQuery(url.query());

        m_version = parts[2];
    }

    // 解析请求头
    for (int i = 1; i < headerLines.size(); i++) {
        parseHeaderLine(QString::fromUtf8(headerLines[i]));
    }
}

void HttpRequest::parseHeaderLine(const QString &line)
{
    int separatorIndex = line.indexOf(':');
    if (separatorIndex > 0) {
        QString key    = line.left(separatorIndex).trimmed();
        QString value  = line.mid(separatorIndex + 1).trimmed();
        m_headers[key] = value;
    }
}
