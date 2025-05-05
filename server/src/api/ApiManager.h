#pragma once

#include "db/QueryExecutor.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

#include <QObject>
#include <QVariantMap>
#include <functional>

class ApiManager: public QObject {
        Q_OBJECT

    public:
        using ApiHandler = std::function<HttpResponse(const HttpRequest &, const QVariantMap &)>;

        static ApiManager &instance();

        // 初始化API
        bool initialize(const QVariantMap &apiConfig);

        // 获取API处理函数
        HttpResponse handleApiRequest(const HttpRequest &request);

    private:
        ApiManager(QObject *parent = nullptr);
        ~ApiManager();

        ApiManager(const ApiManager &)            = delete;
        ApiManager &operator=(const ApiManager &) = delete;

        // 注册API处理函数
        void registerApiHandlers();

        // 标准API处理函数
        HttpResponse handleQueryRequest(const HttpRequest &request, const QVariantMap &config);
        HttpResponse handleQueryByIdRequest(const HttpRequest &request, const QVariantMap &config);
        HttpResponse handleCreateRequest(const HttpRequest &request, const QVariantMap &config);
        HttpResponse handleUpdateRequest(const HttpRequest &request, const QVariantMap &config);
        HttpResponse handleDeleteRequest(const HttpRequest &request, const QVariantMap &config);

        // 自定义API处理函数
        HttpResponse handleHealthCheck(const HttpRequest &request, const QVariantMap &config);

        // 辅助方法 - 请求体解析
        QVariantMap parseRequestBody(const HttpRequest &request) const;

        // 辅助方法 - 成功响应
        HttpResponse createSuccessResponse(const QVariant &data, int statusCode = 200) const;

        // 辅助方法 - 错误响应
        HttpResponse createErrorResponse(const QString &message, int statusCode = 400) const;

        QVariantMap               m_apiConfig;
        QMap<QString, ApiHandler> m_handlers;
        QueryExecutor             m_queryExecutor;
};
