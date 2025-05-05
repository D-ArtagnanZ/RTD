#include "api/ApiManager.h"
#include "db/DatabaseManager.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

ApiManager &ApiManager::instance()
{
    static ApiManager instance;
    return instance;
}

ApiManager::ApiManager(QObject *parent)
    : QObject(parent)
{
}

ApiManager::~ApiManager()
{
}

bool ApiManager::initialize(const QVariantMap &apiConfig)
{
    m_apiConfig = apiConfig;

    // 注册API处理函数
    registerApiHandlers();

    return true;
}

void ApiManager::registerApiHandlers()
{
    // 注册标准API处理函数
    m_handlers["query"] = [this](const HttpRequest &req, const QVariantMap &config) {
        return handleQueryRequest(req, config);
    };

    m_handlers["query_by_id"] = [this](const HttpRequest &req, const QVariantMap &config) {
        return handleQueryByIdRequest(req, config);
    };

    m_handlers["create"] = [this](const HttpRequest &req, const QVariantMap &config) {
        return handleCreateRequest(req, config);
    };

    m_handlers["update"] = [this](const HttpRequest &req, const QVariantMap &config) {
        return handleUpdateRequest(req, config);
    };

    m_handlers["delete"] = [this](const HttpRequest &req, const QVariantMap &config) {
        return handleDeleteRequest(req, config);
    };

    // 注册自定义API处理函数
    m_handlers["health_check"] = [this](const HttpRequest &req, const QVariantMap &config) {
        return handleHealthCheck(req, config);
    };
}

HttpResponse ApiManager::handleApiRequest(const HttpRequest &request)
{
    try {
        QString path   = request.path();
        QString method = request.method();

        // 获取API端点配置
        QVariantMap endpoints = m_apiConfig.value("endpoints").toMap();

        // 查找匹配的API配置
        for (auto it = endpoints.constBegin(); it != endpoints.constEnd(); ++it) {
            QVariantMap endpointGroup = it.value().toMap();

            for (auto configIt = endpointGroup.constBegin(); configIt != endpointGroup.constEnd(); ++configIt) {
                QVariantMap endpointConfig = configIt.value().toMap();

                QString configMethod = endpointConfig.value("method").toString();
                QString configPath   = endpointConfig.value("path").toString();

                // 替换路径参数为正则表达式模式
                QString regexPath = configPath;
                regexPath.replace(QRegularExpression(":[\\w]+"), "([^/]+)");

                // 添加开始和结束边界
                regexPath = "^" + regexPath + "$";

                QRegularExpression      regex(regexPath);
                QRegularExpressionMatch match = regex.match(path);

                if (match.hasMatch() && method == configMethod) {
                    // 提取路径参数
                    QRegularExpression              paramRegex(":(\\w+)");
                    QRegularExpressionMatchIterator paramMatchIt = paramRegex.globalMatch(configPath);
                    int                             captureIndex = 1;

                    QVariantMap pathParams;
                    while (paramMatchIt.hasNext()) {
                        QRegularExpressionMatch paramMatch = paramMatchIt.next();
                        QString                 paramName  = paramMatch.captured(1);
                        QString                 paramValue = match.captured(captureIndex++);
                        pathParams[paramName]              = paramValue;
                    }

                    // 获取处理函数类型
                    QString handlerType;
                    if (endpointConfig.contains("handler")) {
                        handlerType = endpointConfig.value("handler").toString();
                    }
                    else if (configMethod == "GET" && !pathParams.isEmpty()) {
                        handlerType = "query_by_id";
                    }
                    else if (configMethod == "GET") {
                        handlerType = "query";
                    }
                    else if (configMethod == "POST") {
                        handlerType = "create";
                    }
                    else if (configMethod == "PUT") {
                        handlerType = "update";
                    }
                    else if (configMethod == "DELETE") {
                        handlerType = "delete";
                    }

                    // 检查处理函数是否存在
                    if (m_handlers.contains(handlerType)) {
                        // 创建扩展的配置对象，包含路径参数
                        QVariantMap extendedConfig    = endpointConfig;
                        extendedConfig["path_params"] = pathParams;

                        // 调用处理函数
                        return m_handlers[handlerType](request, extendedConfig);
                    }
                }
            }
        }

        // 没有找到匹配的路由
        return createErrorResponse("Not Found", 404);
    }
    catch (const std::exception &e) {
        qCritical() << "API处理异常:" << e.what();
        return createErrorResponse(QString("服务器内部错误: %1").arg(e.what()), 500);
    }
}

HttpResponse ApiManager::handleQueryRequest(const HttpRequest &request, const QVariantMap &config)
{
    try {
        // 获取配置
        QString connectionName = config.value("connection").toString();
        QString sql            = config.value("sql").toString();

        // 获取查询参数
        QVariantMap            params;
        QMap<QString, QString> headers = request.headers();
        for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
            if (it.key().startsWith("X-Param-")) {
                QString paramName = it.key().mid(8);    // 移除"X-Param-"前缀
                params[paramName] = it.value();
            }
        }

        // 执行查询
        QVariantList results = m_queryExecutor.executeQuery(connectionName, sql, params);

        // 返回结果
        return createSuccessResponse(results);
    }
    catch (const std::exception &e) {
        return createErrorResponse(QString("查询失败: %1").arg(e.what()), 500);
    }
}

HttpResponse ApiManager::handleQueryByIdRequest(const HttpRequest &request, const QVariantMap &config)
{
    try {
        // 获取配置
        QString     connectionName = config.value("connection").toString();
        QString     sql            = config.value("sql").toString();
        QVariantMap pathParams     = config.value("path_params").toMap();

        // 创建参数映射
        QVariantMap params;
        for (auto it = pathParams.constBegin(); it != pathParams.constEnd(); ++it) {
            params[it.key()] = it.value();
        }

        // 执行查询
        QVariantMap result = m_queryExecutor.executeSingleRowQuery(connectionName, sql, params);

        // 检查结果是否为空
        if (result.isEmpty()) {
            return createErrorResponse("资源未找到", 404);
        }

        // 返回结果
        return createSuccessResponse(result);
    }
    catch (const std::exception &e) {
        return createErrorResponse(QString("查询失败: %1").arg(e.what()), 500);
    }
}

HttpResponse ApiManager::handleCreateRequest(const HttpRequest &request, const QVariantMap &config)
{
    try {
        // 获取配置
        QString connectionName = config.value("connection").toString();
        QString sql            = config.value("sql").toString();

        // 解析请求体
        QVariantMap bodyParams = parseRequestBody(request);
        if (bodyParams.isEmpty()) {
            return createErrorResponse("无效的请求体", 400);
        }

        // 执行插入
        int id = m_queryExecutor.executeInsert(connectionName, sql, bodyParams);

        // 构建响应
        QVariantMap result = bodyParams;
        result["id"]       = id;

        // 返回结果
        return createSuccessResponse(result, 201);
    }
    catch (const std::exception &e) {
        return createErrorResponse(QString("创建失败: %1").arg(e.what()), 500);
    }
}

HttpResponse ApiManager::handleUpdateRequest(const HttpRequest &request, const QVariantMap &config)
{
    try {
        // 获取配置
        QString     connectionName = config.value("connection").toString();
        QString     sql            = config.value("sql").toString();
        QVariantMap pathParams     = config.value("path_params").toMap();

        // 解析请求体
        QVariantMap bodyParams = parseRequestBody(request);
        if (bodyParams.isEmpty()) {
            return createErrorResponse("无效的请求体", 400);
        }

        // 合并路径参数和请求体参数
        QVariantMap params = bodyParams;
        for (auto it = pathParams.constBegin(); it != pathParams.constEnd(); ++it) {
            params[it.key()] = it.value();
        }

        // 执行更新
        int affectedRows = m_queryExecutor.executeUpdate(connectionName, sql, params);

        // 检查是否更新成功
        if (affectedRows <= 0) {
            return createErrorResponse("资源未找到或未更新", 404);
        }

        // 构建响应
        QVariantMap result      = params;
        result["affected_rows"] = affectedRows;

        // 返回结果
        return createSuccessResponse(result);
    }
    catch (const std::exception &e) {
        return createErrorResponse(QString("更新失败: %1").arg(e.what()), 500);
    }
}

HttpResponse ApiManager::handleDeleteRequest(const HttpRequest &request, const QVariantMap &config)
{
    try {
        // 获取配置
        QString     connectionName = config.value("connection").toString();
        QString     sql            = config.value("sql").toString();
        QVariantMap pathParams     = config.value("path_params").toMap();

        // 执行删除
        int affectedRows = m_queryExecutor.executeUpdate(connectionName, sql, pathParams);

        // 检查是否删除成功
        if (affectedRows <= 0) {
            return createErrorResponse("资源未找到", 404);
        }

        // 构建响应
        QVariantMap result;
        result["deleted"] = true;
        result["id"]      = pathParams.value("id");

        // 返回结果
        return createSuccessResponse(result);
    }
    catch (const std::exception &e) {
        return createErrorResponse(QString("删除失败: %1").arg(e.what()), 500);
    }
}

HttpResponse ApiManager::handleHealthCheck(const HttpRequest &, const QVariantMap &)
{
    QVariantMap healthInfo;
    healthInfo["status"] = "ok";
    healthInfo["time"]   = QDateTime::currentDateTime().toString(Qt::ISODate);

    // 获取数据库连接池状态
    try {
        healthInfo["database"] = DatabaseManager::instance().getPoolsStats();
    }
    catch (const std::exception &) {
        healthInfo["database_status"] = "error";
    }

    return createSuccessResponse(healthInfo);
}

QVariantMap ApiManager::parseRequestBody(const HttpRequest &request) const
{
    QByteArray body = request.body();
    if (body.isEmpty()) {
        return QVariantMap();
    }

    QString contentType = request.headers().value("Content-Type").toLower();

    // 解析JSON
    if (contentType.contains("application/json")) {
        QJsonParseError error;
        QJsonDocument   doc = QJsonDocument::fromJson(body, &error);

        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON解析错误:" << error.errorString();
            return QVariantMap();
        }

        if (doc.isObject()) {
            return doc.object().toVariantMap();
        }
    }
    // 解析表单数据
    else if (contentType.contains("application/x-www-form-urlencoded")) {
        QVariantMap params;
        QUrlQuery   query(QString::fromUtf8(body));

        for (const auto &item: query.queryItems()) {
            params[item.first] = item.second;
        }

        return params;
    }

    return QVariantMap();
}

HttpResponse ApiManager::createSuccessResponse(const QVariant &data, int statusCode) const
{
    QVariantMap response;
    response["success"] = true;
    response["data"]    = data;

    return HttpResponse::fromMap(response, statusCode);
}

HttpResponse ApiManager::createErrorResponse(const QString &message, int statusCode) const
{
    QVariantMap response;
    response["success"] = false;
    response["error"]   = message;

    return HttpResponse::fromMap(response, statusCode);
}
