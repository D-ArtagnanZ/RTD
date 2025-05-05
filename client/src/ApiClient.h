#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QVector>
#include <functional>

/**
 * @brief API客户端类，负责与服务端通信
 */
class ApiClient: public QObject {
        Q_OBJECT

    public:
        using DataCallback   = std::function<void(bool success, const QVector<QVariantMap> &data)>;
        using ObjectCallback = std::function<void(bool success, const QVariantMap &data)>;
        using JsonCallback   = std::function<void(bool success, const QJsonDocument &json)>;

        explicit ApiClient(const QString &baseUrl, QObject *parent = nullptr);

        // 设置API基础URL
        void setBaseUrl(const QString &url);
        void setAuthToken(const QString &token);
        void setSimulationMode(bool enabled);    // 仅保留用于兼容

        // 加载配置
        bool loadConfig(const QString &configPath = "config.json");

        // 基本API方法
        void get(const QString &endpoint, JsonCallback callback);
        void post(const QString &endpoint, const QJsonObject &data, JsonCallback callback);
        void put(const QString &endpoint, const QJsonObject &data, JsonCallback callback);
        void deleteRequest(const QString &endpoint, JsonCallback callback);

        // 派工相关API
        void getLots(DataCallback callback);
        void getEquipment(DataCallback callback);
        void getDispatches(DataCallback callback);
        void createDispatch(const QVariantMap &dispatchData, ObjectCallback callback);
        void updateDispatch(int dispatchId, const QVariantMap &dispatchData, ObjectCallback callback);
        void deleteDispatch(int dispatchId, ObjectCallback callback);

    signals:
        void serverConnectionChanged(bool connected);

    private slots:
        void checkServerConnection();

    private:
        QNetworkAccessManager *m_networkManager;
        QString                m_baseUrl;
        QString                m_authToken;
        QTimer                *m_connectionCheckTimer;
        bool                   m_simulationMode;    // 保留仅用于兼容

        // 辅助方法
        QJsonDocument        parseReply(QNetworkReply *reply, bool &success);
        QVector<QVariantMap> convertJsonArrayToVector(const QJsonArray &array);
        void                 handleNetworkError(QNetworkReply *reply);
};
