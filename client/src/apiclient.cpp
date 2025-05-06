#include "apiclient.h"

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);

    // 从设置中加载服务器URL或使用默认值
    QSettings settings;
    m_serverUrl = settings.value("serverUrl", "http://localhost:8080").toString();

    connect(m_networkManager, &QNetworkAccessManager::finished, this, &ApiClient::handleNetworkReply);
}

ApiClient::~ApiClient()
{
}

void ApiClient::setServerUrl(const QString &url)
{
    m_serverUrl = url;

    // 保存设置
    QSettings settings;
    settings.setValue("serverUrl", url);
}

QString ApiClient::getServerUrl() const
{
    return m_serverUrl;
}

void ApiClient::getEquipmentById(const QString &id)
{
    QJsonObject data;
    data["id"] = id;

    sendPostRequest("/api/getEqpById", data);
    m_activeRequests[m_networkManager->findChild<QNetworkReply *>()] = RequestType::GetEquipment;
}

void ApiClient::getLotById(const QString &id)
{
    QJsonObject data;
    data["id"] = id;

    sendPostRequest("/api/getLotById", data);
    m_activeRequests[m_networkManager->findChild<QNetworkReply *>()] = RequestType::GetLot;
}

void ApiClient::getEquipmentList(int pageNo, int pageSize)
{
    QJsonObject data;
    data["page_no"]   = pageNo;
    data["page_size"] = pageSize;

    sendPostRequest("/api/getEqpList", data);
    m_activeRequests[m_networkManager->findChild<QNetworkReply *>()] = RequestType::GetEquipmentList;
}

void ApiClient::getLotList(int pageNo, int pageSize)
{
    QJsonObject data;
    data["page_no"]   = pageNo;
    data["page_size"] = pageSize;

    sendPostRequest("/api/getLotList", data);
    m_activeRequests[m_networkManager->findChild<QNetworkReply *>()] = RequestType::GetLotList;
}

void ApiClient::getDispatchList(int pageNo, int pageSize)
{
    QJsonObject data;
    data["page_no"]   = pageNo;
    data["page_size"] = pageSize;

    sendPostRequest("/api/getDispatchList", data);
    m_activeRequests[m_networkManager->findChild<QNetworkReply *>()] = RequestType::GetDispatchList;
}

void ApiClient::searchDispatchByEquipmentId(const QString &eqpId)
{
    QJsonObject data;
    data["eqp_id"] = eqpId;

    sendPostRequest("/api/searchDispatchByEqpId", data);
    m_activeRequests[m_networkManager->findChild<QNetworkReply *>()] = RequestType::SearchDispatchByEquipment;
}

void ApiClient::searchDispatchByLotId(const QString &lotId)
{
    QJsonObject data;
    data["lot_id"] = lotId;

    sendPostRequest("/api/searchDispatchByLotId", data);
    m_activeRequests[m_networkManager->findChild<QNetworkReply *>()] = RequestType::SearchDispatchByLot;
}

void ApiClient::createDispatch(const Dispatch &dispatch)
{
    QJsonObject data;
    data["eqp_id"]      = dispatch.eqpId();
    data["lot_id"]      = dispatch.lotId();
    data["status"]      = dispatch.status();
    data["priority"]    = dispatch.priority();
    data["operator_id"] = dispatch.operatorId();

    // 使用所有其他字段
    for (const QString &fieldName: dispatch.fieldNames()) {
        // 跳过已经处理过的标准字段
        QString lowerFieldName = fieldName.toLower();
        if (fieldName != "EQP_ID" && fieldName != "LOT_ID" && fieldName != "STATUS" && fieldName != "PRIORITY" && fieldName != "OPERATOR_ID" && fieldName != "ID") {
            // 将字段名转换为小写，以匹配API接口约定
            data[lowerFieldName] = QJsonValue::fromVariant(dispatch.field(fieldName));
        }
    }

    sendPostRequest("/api/createDispatch", data);
    m_activeRequests[m_networkManager->findChild<QNetworkReply *>()] = RequestType::CreateDispatch;
}

void ApiClient::sendPostRequest(const QString &endpoint, const QJsonObject &data)
{
    QUrl            url(m_serverUrl + endpoint);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(data);
    QByteArray    jsonData = doc.toJson();

    m_networkManager->post(request, jsonData);
}

void ApiClient::handleNetworkReply(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit networkError(reply->errorString());
        m_activeRequests.remove(reply);
        return;
    }

    QByteArray    responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    if (jsonResponse.isNull()) {
        emit networkError("无效的JSON响应");
        m_activeRequests.remove(reply);
        return;
    }

    // 根据请求类型处理响应
    if (m_activeRequests.contains(reply)) {
        RequestType requestType = m_activeRequests[reply];
        m_activeRequests.remove(reply);

        switch (requestType) {
            case RequestType::GetEquipment:
                handleEquipmentResponse(jsonResponse);
                break;
            case RequestType::GetLot:
                handleLotResponse(jsonResponse);
                break;
            case RequestType::GetEquipmentList:
                handleEquipmentListResponse(jsonResponse);
                break;
            case RequestType::GetLotList:
                handleLotListResponse(jsonResponse);
                break;
            case RequestType::GetDispatchList:
                handleDispatchListResponse(jsonResponse);
                break;
            case RequestType::SearchDispatchByEquipment:
                handleDispatchByEquipmentResponse(jsonResponse);
                break;
            case RequestType::SearchDispatchByLot:
                handleDispatchByLotResponse(jsonResponse);
                break;
            case RequestType::CreateDispatch:
                handleCreateDispatchResponse(jsonResponse);
                break;
        }
    }
}

void ApiClient::handleEquipmentResponse(const QJsonDocument &json)
{
    QJsonObject root = json.object();

    if (root.contains("error")) {
        emit equipmentError(root["error"].toString());
        return;
    }

    if (root.contains("data") && root["data"].isArray() && !root["data"].toArray().isEmpty()) {
        QJsonObject equipmentJson = root["data"].toArray().first().toObject();
        Equipment   equipment     = Equipment::fromJson(equipmentJson);
        emit        equipmentReceived(equipment);
    }
    else {
        emit equipmentError("未找到设备信息");
    }
}

void ApiClient::handleLotResponse(const QJsonDocument &json)
{
    QJsonObject root = json.object();

    if (root.contains("error")) {
        emit lotError(root["error"].toString());
        return;
    }

    if (root.contains("data") && root["data"].isArray() && !root["data"].toArray().isEmpty()) {
        QJsonObject lotJson = root["data"].toArray().first().toObject();
        Lot         lot     = Lot::fromJson(lotJson);
        emit        lotReceived(lot);
    }
    else {
        emit lotError("未找到批次信息");
    }
}

void ApiClient::handleEquipmentListResponse(const QJsonDocument &json)
{
    QJsonObject root = json.object();

    if (root.contains("error")) {
        emit equipmentError(root["error"].toString());
        return;
    }

    QList<Equipment> equipmentList;
    int              totalCount = 0;

    if (root.contains("data") && root["data"].isArray()) {
        QJsonArray dataArray = root["data"].toArray();
        totalCount           = dataArray.size();    // 假设服务器未提供总数，使用当前页项目数

        for (const QJsonValue &value: dataArray) {
            if (value.isObject()) {
                Equipment equipment = Equipment::fromJson(value.toObject());
                equipmentList.append(equipment);
            }
        }
    }

    emit equipmentListReceived(equipmentList, totalCount);
}

void ApiClient::handleLotListResponse(const QJsonDocument &json)
{
    QJsonObject root = json.object();

    if (root.contains("error")) {
        emit lotError(root["error"].toString());
        return;
    }

    QList<Lot> lotList;
    int        totalCount = 0;

    if (root.contains("data") && root["data"].isArray()) {
        QJsonArray dataArray = root["data"].toArray();
        totalCount           = dataArray.size();    // 假设服务器未提供总数，使用当前页项目数

        for (const QJsonValue &value: dataArray) {
            if (value.isObject()) {
                Lot lot = Lot::fromJson(value.toObject());
                lotList.append(lot);
            }
        }
    }

    emit lotListReceived(lotList, totalCount);
}

void ApiClient::handleDispatchListResponse(const QJsonDocument &json)
{
    QJsonObject root = json.object();

    if (root.contains("error")) {
        emit dispatchError(root["error"].toString());
        return;
    }

    QList<Dispatch> dispatchList;
    int             totalCount = 0;

    if (root.contains("data") && root["data"].isArray()) {
        QJsonArray dataArray = root["data"].toArray();
        totalCount           = dataArray.size();    // 假设服务器未提供总数，使用当前页项目数

        for (const QJsonValue &value: dataArray) {
            if (value.isObject()) {
                Dispatch dispatch = Dispatch::fromJson(value.toObject());
                dispatchList.append(dispatch);
            }
        }
    }

    emit dispatchListReceived(dispatchList, totalCount);
}

void ApiClient::handleDispatchByEquipmentResponse(const QJsonDocument &json)
{
    QJsonObject root = json.object();

    if (root.contains("error")) {
        emit dispatchError(root["error"].toString());
        return;
    }

    QList<Dispatch> dispatchList;

    if (root.contains("data") && root["data"].isArray()) {
        QJsonArray dataArray = root["data"].toArray();

        for (const QJsonValue &value: dataArray) {
            if (value.isObject()) {
                Dispatch dispatch = Dispatch::fromJson(value.toObject());
                dispatchList.append(dispatch);
            }
        }
    }

    emit dispatchByEquipmentReceived(dispatchList);
}

void ApiClient::handleDispatchByLotResponse(const QJsonDocument &json)
{
    QJsonObject root = json.object();

    if (root.contains("error")) {
        emit dispatchError(root["error"].toString());
        return;
    }

    QList<Dispatch> dispatchList;

    if (root.contains("data") && root["data"].isArray()) {
        QJsonArray dataArray = root["data"].toArray();

        for (const QJsonValue &value: dataArray) {
            if (value.isObject()) {
                Dispatch dispatch = Dispatch::fromJson(value.toObject());
                dispatchList.append(dispatch);
            }
        }
    }

    emit dispatchByLotReceived(dispatchList);
}

void ApiClient::handleCreateDispatchResponse(const QJsonDocument &json)
{
    QJsonObject root = json.object();

    if (root.contains("error")) {
        emit dispatchCreated(false, root["error"].toString());
        return;
    }

    emit dispatchCreated(true, "调度任务创建成功");
}
