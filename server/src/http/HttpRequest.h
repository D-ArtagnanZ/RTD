#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QUrlQuery>

class HttpRequest {
    public:
        HttpRequest();
        HttpRequest(const QByteArray &requestData);

        // 获取请求信息
        QString                method() const;
        QString                path() const;
        QString                version() const;
        QMap<QString, QString> headers() const;
        QByteArray             body() const;

        // 参数访问
        QString getQueryParameter(const QString &name) const;
        QString getPathParameter(const QString &name) const;
        QString getBodyParameter(const QString &name) const;

        // 设置路径参数（由HttpServer调用）
        void setPathParameters(const QMap<QString, QString> &params);

    private:
        void parseRequest(const QByteArray &data);
        void parseHeaderLine(const QString &line);

        QString                m_method;
        QString                m_path;
        QString                m_version;
        QMap<QString, QString> m_headers;
        QByteArray             m_body;
        QUrlQuery              m_query;
        QMap<QString, QString> m_pathParams;
};
