#pragma once

#include "models.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSettings>
#include <QUrlQuery>

class ApiClient: public QObject {
        Q_OBJECT

    public:
        explicit ApiClient(QObject *parent = nullptr);
        ~ApiClient();

        // 设置服务器URL
        void    setServerUrl(const QString &url);
        QString getServerUrl() const;

        // API调用
        void getEquipmentById(const QString &id);
        void getLotById(const QString &id);
        void getEquipmentList(int pageNo, int pageSize);
        void getLotList(int pageNo, int pageSize);
        void getDispatchList(int pageNo, int pageSize);
        void searchDispatchByEquipmentId(const QString &eqpId);
        void searchDispatchByLotId(const QString &lotId);    // 添加新方法
        void createDispatch(const Dispatch &dispatch);

    signals:
        // 设备相关信号
        void equipmentReceived(const Equipment &equipment);
        void equipmentListReceived(const QList<Equipment> &equipmentList, int totalCount);
        void equipmentError(const QString &errorMessage);

        // 批次相关信号
        void lotReceived(const Lot &lot);
        void lotListReceived(const QList<Lot> &lotList, int totalCount);
        void lotError(const QString &errorMessage);

        // 调度相关信号
        void dispatchListReceived(const QList<Dispatch> &dispatchList, int totalCount);
        void dispatchByEquipmentReceived(const QList<Dispatch> &dispatchList);
        void dispatchByLotReceived(const QList<Dispatch> &dispatchList);    // 添加新信号
        void dispatchCreated(bool success, const QString &message);
        void dispatchError(const QString &errorMessage);

        // 通用错误信号
        void networkError(const QString &errorMessage);

    private slots:
        void handleNetworkReply(QNetworkReply *reply);

    private:
        QNetworkAccessManager *m_networkManager;
        QString                m_serverUrl;

        // 发送POST请求
        void sendPostRequest(const QString &endpoint, const QJsonObject &data);

        // 处理不同类型的响应
        void handleEquipmentResponse(const QJsonDocument &json);
        void handleLotResponse(const QJsonDocument &json);
        void handleEquipmentListResponse(const QJsonDocument &json);
        void handleLotListResponse(const QJsonDocument &json);
        void handleDispatchListResponse(const QJsonDocument &json);
        void handleDispatchByEquipmentResponse(const QJsonDocument &json);
        void handleDispatchByLotResponse(const QJsonDocument &json);    // 添加新处理方法
        void handleCreateDispatchResponse(const QJsonDocument &json);

        // 用于跟踪请求类型
        enum class RequestType {
            GetEquipment,
            GetLot,
            GetEquipmentList,
            GetLotList,
            GetDispatchList,
            SearchDispatchByEquipment,
            SearchDispatchByLot,    // 添加新请求类型
            CreateDispatch
        };

        QMap<QNetworkReply *, RequestType> m_activeRequests;
};
