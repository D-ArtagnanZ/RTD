#include "ApiClient.h"

#include <QDateTime>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QSettings>
#include <QUrlQuery>

// 构造函数
ApiClient::ApiClient(const QString &baseUrl, QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_baseUrl(baseUrl)
    , m_connectionCheckTimer(new QTimer(this))
    , m_simulationMode(false)    // 默认不使用模拟模式
{
    // 设置连接检查定时器
    m_connectionCheckTimer->setInterval(30000);    // 每30秒检查一次
    connect(m_connectionCheckTimer, &QTimer::timeout, this, &ApiClient::checkServerConnection);
    m_connectionCheckTimer->start();

    // 初始检查
    QTimer::singleShot(100, this, &ApiClient::checkServerConnection);
}

void ApiClient::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
    // 更改URL后立即检查连接
    QTimer::singleShot(100, this, &ApiClient::checkServerConnection);
}

void ApiClient::setAuthToken(const QString &token)
{
    m_authToken = token;
}

void ApiClient::setSimulationMode(bool enabled)
{
    // 兼容旧代码，但我们现在总是使用真实API
    m_simulationMode = false;
    qDebug() << "模拟模式已弃用，总是连接到真实API";
}

bool ApiClient::loadConfig(const QString &configPath)
{
    QFileInfo configFile(configPath);
    if (!configFile.exists() || !configFile.isReadable()) {
        qWarning() << "配置文件不存在或不可读:" << configPath;
        return false;
    }

    try {
        // 使用QSettings读取JSON配置
        QSettings settings(configPath, QSettings::IniFormat);

        // 读取服务器URL
        if (settings.contains("serverUrl")) {
            setBaseUrl(settings.value("serverUrl").toString());
            qDebug() << "从配置加载服务器URL:" << m_baseUrl;
        }

        // 读取认证令牌(如果有)
        if (settings.contains("authToken")) {
            setAuthToken(settings.value("authToken").toString());
        }

        return true;
    }
    catch (const std::exception &e) {
        qWarning() << "加载配置文件失败:" << e.what();
        return false;
    }
}

// REST API实现 - GET请求
void ApiClient::get(const QString &endpoint, JsonCallback callback)
{
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 添加认证头(如果有)
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }

    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply, callback]() {
        bool          success      = false;
        QJsonDocument jsonResponse = parseReply(reply, success);

        // 执行回调
        callback(success, jsonResponse);

        // 处理可能的错误
        if (!success) {
            handleNetworkError(reply);
        }

        // 释放资源
        reply->deleteLater();
    });
}

// REST API实现 - POST请求
void ApiClient::post(const QString &endpoint, const QJsonObject &data, JsonCallback callback)
{
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 添加认证头(如果有)
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }

    QJsonDocument doc(data);
    QByteArray    jsonData = doc.toJson();

    QNetworkReply *reply = m_networkManager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, [this, reply, callback]() {
        bool          success      = false;
        QJsonDocument jsonResponse = parseReply(reply, success);

        // 执行回调
        callback(success, jsonResponse);

        // 处理可能的错误
        if (!success) {
            handleNetworkError(reply);
        }

        // 释放资源
        reply->deleteLater();
    });
}

// REST API实现 - PUT请求
void ApiClient::put(const QString &endpoint, const QJsonObject &data, JsonCallback callback)
{
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 添加认证头(如果有)
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }

    QJsonDocument doc(data);
    QByteArray    jsonData = doc.toJson();

    QNetworkReply *reply = m_networkManager->put(request, jsonData);

    connect(reply, &QNetworkReply::finished, [this, reply, callback]() {
        bool          success      = false;
        QJsonDocument jsonResponse = parseReply(reply, success);

        // 执行回调
        callback(success, jsonResponse);

        // 处理可能的错误
        if (!success) {
            handleNetworkError(reply);
        }

        // 释放资源
        reply->deleteLater();
    });
}

// REST API实现 - DELETE请求
void ApiClient::deleteRequest(const QString &endpoint, JsonCallback callback)
{
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 添加认证头(如果有)
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }

    QNetworkReply *reply = m_networkManager->deleteResource(request);

    connect(reply, &QNetworkReply::finished, [this, reply, callback]() {
        bool          success      = false;
        QJsonDocument jsonResponse = parseReply(reply, success);

        // 执行回调
        callback(success, jsonResponse);

        // 处理可能的错误
        if (!success) {
            handleNetworkError(reply);
        }

        // 释放资源
        reply->deleteLater();
    });
}

// API方法 - 获取产品批次
void ApiClient::getLots(DataCallback callback)
{
    get("/api/lots", [this, callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject() && doc["success"].toBool()) {
            QJsonArray data = doc["data"].toArray();
            callback(true, convertJsonArrayToVector(data));
        }
        else {
            callback(false, QVector<QVariantMap>());
        }
    });
}

// API方法 - 获取设备
void ApiClient::getEquipment(DataCallback callback)
{
    get("/api/equipment", [this, callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject() && doc["success"].toBool()) {
            QJsonArray data = doc["data"].toArray();
            callback(true, convertJsonArrayToVector(data));
        }
        else {
            callback(false, QVector<QVariantMap>());
        }
    });
}

// API方法 - 获取派工单
void ApiClient::getDispatches(DataCallback callback)
{
    get("/api/dispatches", [this, callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject() && doc["success"].toBool()) {
            QJsonArray data = doc["data"].toArray();
            callback(true, convertJsonArrayToVector(data));
        }
        else {
            callback(false, QVector<QVariantMap>());
        }
    });
}

// API方法 - 创建派工单
void ApiClient::createDispatch(const QVariantMap &dispatchData, ObjectCallback callback)
{
    // 将QVariantMap转换为QJsonObject
    QJsonObject jsonData;
    for (auto it = dispatchData.constBegin(); it != dispatchData.constEnd(); ++it) {
        jsonData[it.key()] = QJsonValue::fromVariant(it.value());
    }

    post("/api/dispatches", jsonData, [callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject() && doc["success"].toBool()) {
            QVariantMap data = doc["data"].toObject().toVariantMap();
            callback(true, data);
        }
        else {
            callback(false, QVariantMap());
        }
    });
}

// API方法 - 更新派工单
void ApiClient::updateDispatch(int dispatchId, const QVariantMap &dispatchData, ObjectCallback callback)
{
    // 将QVariantMap转换为QJsonObject
    QJsonObject jsonData;
    for (auto it = dispatchData.constBegin(); it != dispatchData.constEnd(); ++it) {
        jsonData[it.key()] = QJsonValue::fromVariant(it.value());
    }

    put(QString("/api/dispatches/%1").arg(dispatchId), jsonData, [callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject() && doc["success"].toBool()) {
            QVariantMap data = doc["data"].toObject().toVariantMap();
            callback(true, data);
        }
        else {
            callback(false, QVariantMap());
        }
    });
}

// API方法 - 删除派工单
void ApiClient::deleteDispatch(int dispatchId, ObjectCallback callback)
{
    deleteRequest(QString("/api/dispatches/%1").arg(dispatchId), [callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject() && doc["success"].toBool()) {
            QVariantMap data = doc["data"].toObject().toVariantMap();
            callback(true, data);
        }
        else {
            callback(false, QVariantMap());
        }
    });
}

// 检查服务器连接状态
void ApiClient::checkServerConnection()
{
    QNetworkRequest request(QUrl(m_baseUrl + "/api/health"));
    QNetworkReply  *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        bool connected = (reply->error() == QNetworkReply::NoError);
        emit serverConnectionChanged(connected);

        if (!connected) {
            qWarning() << "无法连接到服务器:" << reply->errorString();
        }

        reply->deleteLater();
    });
}

// 辅助方法 - 解析网络响应
QJsonDocument ApiClient::parseReply(QNetworkReply *reply, bool &success)
{
    success = false;

    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError) {
        return QJsonDocument();
    }

    // 读取响应数据
    QByteArray data = reply->readAll();

    // 解析JSON
    QJsonParseError parseError;
    QJsonDocument   doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误:" << parseError.errorString();
        return QJsonDocument();
    }

    success = true;
    return doc;
}

// 辅助方法 - 将JSON数组转换为QVector<QVariantMap>
QVector<QVariantMap> ApiClient::convertJsonArrayToVector(const QJsonArray &array)
{
    QVector<QVariantMap> result;

    for (const QJsonValue &value: array) {
        if (value.isObject()) {
            result.append(value.toObject().toVariantMap());
        }
    }

    return result;
}

// 辅助方法 - 处理网络错误
void ApiClient::handleNetworkError(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        return;
    }

    // 只记录错误，不显示给用户
    qWarning() << "网络请求错误:" << reply->error() << reply->errorString();
    qWarning() << "URL:" << reply->url().toString();
}