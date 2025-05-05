#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>

class HttpResponse {
    public:
        HttpResponse(int statusCode = 200);

        // 设置状态码
        void setStatusCode(int code);

        // 设置头部字段
        void setHeader(const QString &name, const QString &value);

        // 设置响应体
        void setBody(const QByteArray &body);

        // 获取状态码
        int statusCode() const;

        // 转换为字节数组
        QByteArray toByteArray() const;

        // 便捷方法 - 创建JSON响应
        static HttpResponse fromJson(const QByteArray &json, int statusCode = 200);
        static HttpResponse fromMap(const QMap<QString, QVariant> &map, int statusCode = 200);

    private:
        // 获取状态文本
        static QString getStatusText(int code);

        int                    m_statusCode;
        QMap<QString, QString> m_headers;
        QByteArray             m_body;
};
