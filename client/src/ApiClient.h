#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QVector>
#include <functional>

/**
 * @brief API客户端类，负责与服务端通信
 */
class ApiClient: public QObject {
        Q_OBJECT

    public:
        // 回调函数类型定义
        using DataCallback   = std::function<void(bool success, const QVector<QVariantMap> &data)>;
        using ObjectCallback = std::function<void(bool success, const QVariantMap &data)>;

        explicit ApiClient(const QString &baseUrl, QObject *parent = nullptr);

        // 设置API基础URL
        void setBaseUrl(const QString &url);
        void setAuthToken(const QString &token);
        void setSimulationMode(bool enabled);

        // 基本API方法
        void getLots(DataCallback callback);
        void getEquipment(DataCallback callback);
        void getDispatches(DataCallback callback);

        // 派工相关API
        void createDispatch(const QVariantMap &dispatchData, ObjectCallback callback);
        void updateDispatch(int dispatchId, const QVariantMap &dispatchData, ObjectCallback callback);
        void deleteDispatch(int dispatchId, ObjectCallback callback);

    signals:
        void connectionStatusChanged(bool connected);

    private slots:
        void checkServerConnection();

    private:
        // 模拟API请求的方法（用于开发阶段）
        void simulateGetLots(DataCallback callback);
        void simulateGetEquipment(DataCallback callback);
        void simulateGetDispatches(DataCallback callback);

        // 网络请求方法
        void get(const QString &endpoint, std::function<void(bool, const QJsonDocument &)> callback);
        void post(const QString &endpoint, const QJsonObject &data, std::function<void(bool, const QJsonDocument &)> callback);
        void put(const QString &endpoint, const QJsonObject &data, std::function<void(bool, const QJsonDocument &)> callback);
        void deleteRequest(const QString &endpoint, std::function<void(bool, const QJsonDocument &)> callback);

        // 模拟数据生成
        QJsonArray simulateLotData();
        QJsonArray simulateEquipmentData();
        QJsonArray simulateDispatchData();

        QNetworkAccessManager m_networkManager;
        QString               m_baseUrl;
        QString               m_authToken;
        QTimer               *m_connectionCheckTimer;
        bool                  m_isSimulationMode;
};
