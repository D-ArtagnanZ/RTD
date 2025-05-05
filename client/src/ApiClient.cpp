#include "ApiClient.h"
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QTimer>
#include <QUrlQuery>

ApiClient::ApiClient(const QString &baseUrl, QObject *parent)
    : QObject(parent)
    , m_baseUrl(baseUrl)
    , m_isSimulationMode(true)    // 开发阶段使用模拟模式
{
    // 初始化网络管理器
    m_connectionCheckTimer = new QTimer(this);
    connect(m_connectionCheckTimer, &QTimer::timeout, this, &ApiClient::checkServerConnection);
    m_connectionCheckTimer->start(30000);    // 30秒检查一次连接
}

void ApiClient::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
}

void ApiClient::setAuthToken(const QString &token)
{
    m_authToken = token;
}

void ApiClient::setSimulationMode(bool enabled)
{
    m_isSimulationMode = enabled;
}

// REST API实现 - GET请求
void ApiClient::get(const QString &endpoint, std::function<void(bool, const QJsonDocument &)> callback)
{
    if (m_isSimulationMode) {
        // 在模拟模式下，根据端点返回模拟数据
        QTimer::singleShot(300, this, [this, endpoint, callback]() {
            if (endpoint.contains("lots")) {
                QJsonDocument doc(simulateLotData());
                callback(true, doc);
            }
            else if (endpoint.contains("equipment")) {
                QJsonDocument doc(simulateEquipmentData());
                callback(true, doc);
            }
            else if (endpoint.contains("dispatches")) {
                QJsonDocument doc(simulateDispatchData());
                callback(true, doc);
            }
            else {
                callback(false, QJsonDocument());
            }
        });
        return;
    }

    // 实际网络请求
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 添加认证头（如果有）
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }

    QNetworkReply *reply = m_networkManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            callback(true, doc);
        }
        else {
            qWarning() << "Network error:" << reply->errorString();
            callback(false, QJsonDocument());
        }
    });
}

// REST API实现 - POST请求
void ApiClient::post(const QString &endpoint, const QJsonObject &data, std::function<void(bool, const QJsonDocument &)> callback)
{
    if (m_isSimulationMode) {
        // 模拟成功响应
        QTimer::singleShot(300, this, [callback, data]() {
            QJsonObject response = data;
            response["id"]       = QRandomGenerator::global()->bounded(1000, 9999);
            response["success"]  = true;
            callback(true, QJsonDocument(response));
        });
        return;
    }

    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }

    QByteArray     jsonData = QJsonDocument(data).toJson();
    QNetworkReply *reply    = m_networkManager.post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            callback(true, doc);
        }
        else {
            qWarning() << "Network error:" << reply->errorString();
            callback(false, QJsonDocument());
        }
    });
}

// REST API实现 - PUT请求
void ApiClient::put(const QString &endpoint, const QJsonObject &data, std::function<void(bool, const QJsonDocument &)> callback)
{
    if (m_isSimulationMode) {
        // 模拟成功响应
        QTimer::singleShot(300, this, [callback, data]() {
            QJsonObject response = data;
            response["updated"]  = true;
            callback(true, QJsonDocument(response));
        });
        return;
    }

    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }

    QByteArray     jsonData = QJsonDocument(data).toJson();
    QNetworkReply *reply    = m_networkManager.put(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            callback(true, doc);
        }
        else {
            qWarning() << "Network error:" << reply->errorString();
            callback(false, QJsonDocument());
        }
    });
}

// REST API实现 - DELETE请求
void ApiClient::deleteRequest(const QString &endpoint, std::function<void(bool, const QJsonDocument &)> callback)
{
    if (m_isSimulationMode) {
        // 模拟成功响应
        QTimer::singleShot(300, this, [callback]() {
            QJsonObject response;
            response["deleted"] = true;
            callback(true, QJsonDocument(response));
        });
        return;
    }

    QNetworkRequest request(QUrl(m_baseUrl + endpoint));

    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }

    QNetworkReply *reply = m_networkManager.deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            callback(true, doc);
        }
        else {
            qWarning() << "Network error:" << reply->errorString();
            callback(false, QJsonDocument());
        }
    });
}

// API方法 - 获取产品批次
void ApiClient::getLots(DataCallback callback)
{
    // 开发阶段使用模拟数据
    simulateGetLots(callback);
}

// API方法 - 获取设备
void ApiClient::getEquipment(DataCallback callback)
{
    // 开发阶段使用模拟数据
    simulateGetEquipment(callback);
}

// API方法 - 获取派工单
void ApiClient::getDispatches(DataCallback callback)
{
    // 开发阶段使用模拟数据
    simulateGetDispatches(callback);
}

// API方法 - 创建派工单
void ApiClient::createDispatch(const QVariantMap &dispatchData, ObjectCallback callback)
{
    if (m_isSimulationMode) {
        QTimer::singleShot(300, this, [callback, dispatchData]() {
            QVariantMap response = dispatchData;
            response["id"]       = QRandomGenerator::global()->bounded(1000, 9999);
            callback(true, response);
        });
        return;
    }

    QJsonObject jsonData = QJsonObject::fromVariantMap(dispatchData);
    post("/api/dispatches", jsonData, [callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject()) {
            callback(true, doc.object().toVariantMap());
        }
        else {
            callback(false, QVariantMap());
        }
    });
}

// API方法 - 更新派工单
void ApiClient::updateDispatch(int dispatchId, const QVariantMap &dispatchData, ObjectCallback callback)
{
    if (m_isSimulationMode) {
        QTimer::singleShot(300, this, [callback, dispatchId, dispatchData]() {
            QVariantMap response = dispatchData;
            response["id"]       = dispatchId;
            response["updated"]  = true;
            callback(true, response);
        });
        return;
    }

    QJsonObject jsonData = QJsonObject::fromVariantMap(dispatchData);
    put(QString("/api/dispatches/%1").arg(dispatchId), jsonData, [callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject()) {
            callback(true, doc.object().toVariantMap());
        }
        else {
            callback(false, QVariantMap());
        }
    });
}

// API方法 - 删除派工单
void ApiClient::deleteDispatch(int dispatchId, ObjectCallback callback)
{
    if (m_isSimulationMode) {
        QTimer::singleShot(300, this, [callback, dispatchId]() {
            QVariantMap response;
            response["id"]      = dispatchId;
            response["deleted"] = true;
            callback(true, response);
        });
        return;
    }

    deleteRequest(QString("/api/dispatches/%1").arg(dispatchId), [callback](bool success, const QJsonDocument &doc) {
        if (success && doc.isObject()) {
            callback(true, doc.object().toVariantMap());
        }
        else {
            callback(false, QVariantMap());
        }
    });
}

// 检查服务器连接状态
void ApiClient::checkServerConnection()
{
    if (m_isSimulationMode) {
        emit connectionStatusChanged(true);
        return;
    }

    QNetworkRequest request(QUrl(m_baseUrl + "/api/health"));
    QNetworkReply  *reply = m_networkManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        bool connected = (reply->error() == QNetworkReply::NoError);
        emit connectionStatusChanged(connected);
    });
}

// 模拟数据生成方法
QJsonArray ApiClient::simulateLotData()
{
    QJsonArray  lots;
    QStringList statuses = {"WAITING", "PROCESSING", "COMPLETED", "HOLD", "ABORTED"};
    QStringList pdids    = {"PD001", "PD002", "PD003", "PD004", "PD005"};
    QStringList flows    = {"FLOW_A", "FLOW_B", "FLOW_C"};
    QStringList opers    = {"TEST", "QC", "PACKAGE"};
    QDateTime   now      = QDateTime::currentDateTime();

    for (int i = 1; i <= 20; ++i) {
        QJsonObject lot;
        lot["LOT_ID"]        = QString("LOT-%1").arg(i, 6, 10, QChar('0'));
        lot["STATUS"]        = statuses[QRandomGenerator::global()->bounded(statuses.size())];
        lot["PDID"]          = pdids[QRandomGenerator::global()->bounded(pdids.size())];
        lot["FLOW"]          = flows[QRandomGenerator::global()->bounded(flows.size())];
        lot["OPER"]          = opers[QRandomGenerator::global()->bounded(opers.size())];
        lot["BALL_COUNT"]    = QRandomGenerator::global()->bounded(500, 2000);
        lot["CREATION_TIME"] = now.addDays(-QRandomGenerator::global()->bounded(30)).toString(Qt::ISODate);
        lot["UPDATE_TIME"]   = now.addSecs(QRandomGenerator::global()->bounded(86400 * 5)).toString(Qt::ISODate);
        lots.append(lot);
    }
    return lots;
}

// 其它模拟方法和辅助函数
QJsonArray ApiClient::simulateEquipmentData()
{
    QJsonArray  equipment;
    QStringList statuses = {"IDLE", "RUNNING", "MAINTENANCE", "ERROR", "OFFLINE"};
    QStringList kitSizes = {"SMALL", "MEDIUM", "LARGE", "CUSTOM"};
    QDateTime   now      = QDateTime::currentDateTime();

    for (int i = 1; i <= 15; ++i) {
        QJsonObject equip;
        equip["EQP_ID"]           = QString("EQP-%1").arg(i, 3, 10, QChar('0'));
        equip["STATUS"]           = statuses[QRandomGenerator::global()->bounded(statuses.size())];
        equip["TEMPERATURE"]      = 20.0 + QRandomGenerator::global()->bounded(60.0);
        equip["BALL_COUNT"]       = QRandomGenerator::global()->bounded(1000, 5000);
        equip["KIT_SIZE"]         = kitSizes[QRandomGenerator::global()->bounded(kitSizes.size())];
        equip["MAINTENANCE_TIME"] = now.addDays(QRandomGenerator::global()->bounded(-10, 30)).toString("yyyy-MM-dd");
        equipment.append(equip);
    }
    return equipment;
}

QJsonArray ApiClient::simulateDispatchData()
{
    QJsonArray  dispatches;
    QStringList statuses = {"SCHEDULED", "RUNNING", "COMPLETED", "CANCELLED"};
    QDateTime   now      = QDateTime::currentDateTime();

    for (int i = 1; i <= 25; ++i) {
        QJsonObject dispatch;
        dispatch["ID"]             = i;
        dispatch["LOT_ID"]         = QString("LOT-%1").arg(QRandomGenerator::global()->bounded(1, 21), 6, 10, QChar('0'));
        dispatch["EQP_ID"]         = QString("EQP-%1").arg(QRandomGenerator::global()->bounded(1, 16), 3, 10, QChar('0'));
        dispatch["PRIORITY"]       = QRandomGenerator::global()->bounded(1, 6);
        dispatch["DISPATCH_TIME"]  = now.addSecs(-QRandomGenerator::global()->bounded(3600 * 48)).toString(Qt::ISODate);
        dispatch["EXPECTED_START"] = now.addSecs(QRandomGenerator::global()->bounded(3600, 3600 * 6)).toString(Qt::ISODate);
        dispatch["EXPECTED_END"]   = now.addSecs(QRandomGenerator::global()->bounded(3600, 3600 * 8)).toString(Qt::ISODate);
        dispatch["STATUS"]         = statuses[QRandomGenerator::global()->bounded(statuses.size())];
        dispatches.append(dispatch);
    }
    return dispatches;
}

// 实现模拟API请求的方法
void ApiClient::simulateGetLots(DataCallback callback)
{
    // 模拟网络延迟
    QTimer::singleShot(300, this, [callback]() {
        QVector<QVariantMap> lots;

        // 生成模拟数据
        QStringList statuses = {"WAITING", "PROCESSING", "COMPLETED", "HOLD", "ABORTED"};
        QStringList pdids    = {"PD001", "PD002", "PD003", "PD004", "PD005"};
        QStringList flows    = {"FLOW_A", "FLOW_B", "FLOW_C"};
        QStringList opers    = {"TEST", "QC", "PACKAGE"};

        QDateTime now = QDateTime::currentDateTime();

        // 生成20条模拟数据
        for (int i = 1; i <= 20; ++i) {
            QVariantMap lot;

            // 随机选择状态和其他属性
            QString status    = statuses[QRandomGenerator::global()->bounded(statuses.size())];
            QString pdid      = pdids[QRandomGenerator::global()->bounded(pdids.size())];
            QString flow      = flows[QRandomGenerator::global()->bounded(flows.size())];
            QString oper      = opers[QRandomGenerator::global()->bounded(opers.size())];
            int     ballCount = QRandomGenerator::global()->bounded(500, 2000);

            // 生成随机时间
            QDateTime creationTime = now.addDays(-QRandomGenerator::global()->bounded(30));
            QDateTime updateTime   = creationTime.addSecs(QRandomGenerator::global()->bounded(86400 * 5));

            // 填充数据
            lot["LOT_ID"]        = QString("LOT-%1").arg(i, 6, 10, QChar('0'));
            lot["STATUS"]        = status;
            lot["PDID"]          = pdid;
            lot["FLOW"]          = flow;
            lot["OPER"]          = oper;
            lot["BALL_COUNT"]    = ballCount;
            lot["CREATION_TIME"] = creationTime.toString(Qt::ISODate);
            lot["UPDATE_TIME"]   = updateTime.toString(Qt::ISODate);

            lots.append(lot);
        }

        callback(true, lots);
    });
}

void ApiClient::simulateGetEquipment(DataCallback callback)
{
    // 模拟网络延迟
    QTimer::singleShot(300, this, [callback]() {
        QVector<QVariantMap> equipment;

        // 生成模拟数据
        QStringList statuses = {"IDLE", "RUNNING", "MAINTENANCE", "ERROR", "OFFLINE"};
        QStringList kitSizes = {"SMALL", "MEDIUM", "LARGE", "CUSTOM"};

        QDateTime now = QDateTime::currentDateTime();

        // 生成15条模拟数据
        for (int i = 1; i <= 15; ++i) {
            QVariantMap equip;

            // 随机选择状态和其他属性
            QString status      = statuses[QRandomGenerator::global()->bounded(statuses.size())];
            QString kitSize     = kitSizes[QRandomGenerator::global()->bounded(kitSizes.size())];
            double  temperature = 20.0 + QRandomGenerator::global()->bounded(60.0);    // 20-80度
            int     ballCount   = QRandomGenerator::global()->bounded(1000, 5000);

            // 生成随机维护时间
            QDateTime maintenanceTime = now.addDays(QRandomGenerator::global()->bounded(-10, 30));

            // 填充数据
            equip["EQP_ID"]           = QString("EQP-%1").arg(i, 3, 10, QChar('0'));
            equip["STATUS"]           = status;
            equip["TEMPERATURE"]      = temperature;
            equip["BALL_COUNT"]       = ballCount;
            equip["KIT_SIZE"]         = kitSize;
            equip["MAINTENANCE_TIME"] = maintenanceTime.toString("yyyy-MM-dd");

            equipment.append(equip);
        }

        callback(true, equipment);
    });
}

void ApiClient::simulateGetDispatches(DataCallback callback)
{
    // 模拟网络延迟
    QTimer::singleShot(300, this, [callback]() {
        QVector<QVariantMap> dispatches;

        // 生成模拟数据
        QStringList statuses = {"SCHEDULED", "RUNNING", "COMPLETED", "CANCELLED"};

        QDateTime now = QDateTime::currentDateTime();

        // 生成25条模拟数据
        for (int i = 1; i <= 25; ++i) {
            QVariantMap dispatch;

            // 随机选择状态和其他属性
            QString status   = statuses[QRandomGenerator::global()->bounded(statuses.size())];
            int     priority = QRandomGenerator::global()->bounded(1, 6);    // 1-5优先级

            // 批次ID和设备ID
            QString lotId = QString("LOT-%1").arg(QRandomGenerator::global()->bounded(1, 21), 6, 10, QChar('0'));
            QString eqpId = QString("EQP-%1").arg(QRandomGenerator::global()->bounded(1, 16), 3, 10, QChar('0'));

            // 生成随机时间
            QDateTime dispatchTime  = now.addSecs(-QRandomGenerator::global()->bounded(3600 * 48));
            QDateTime expectedStart = dispatchTime.addSecs(QRandomGenerator::global()->bounded(3600, 3600 * 6));
            QDateTime expectedEnd   = expectedStart.addSecs(QRandomGenerator::global()->bounded(3600, 3600 * 8));

            // 填充数据
            dispatch["ID"]             = i;
            dispatch["LOT_ID"]         = lotId;
            dispatch["EQP_ID"]         = eqpId;
            dispatch["PRIORITY"]       = priority;
            dispatch["DISPATCH_TIME"]  = dispatchTime.toString(Qt::ISODate);
            dispatch["EXPECTED_START"] = expectedStart.toString(Qt::ISODate);
            dispatch["EXPECTED_END"]   = expectedEnd.toString(Qt::ISODate);
            dispatch["STATUS"]         = status;

            dispatches.append(dispatch);
        }

        callback(true, dispatches);
    });
}