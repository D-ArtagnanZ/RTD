#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

class ConfigManager: public QObject {
        Q_OBJECT

    public:
        static ConfigManager &instance();

        // 加载配置
        bool loadConfigs(const QString &serverConfigPath = "config/server.json");

        // 获取配置
        QVariantMap getServerConfig() const;
        QVariantMap getDatabaseConfig() const;
        QVariantMap getApiConfig() const;

        // 获取特定配置项
        QVariant getConfigValue(const QString &section, const QString &key, const QVariant &defaultValue = QVariant()) const;

    private:
        ConfigManager(QObject *parent = nullptr);
        ~ConfigManager();

        ConfigManager(const ConfigManager &)            = delete;
        ConfigManager &operator=(const ConfigManager &) = delete;

        // 加载JSON文件
        QVariantMap loadJsonFile(const QString &path) const;

        // 配置数据
        QVariantMap m_serverConfig;
        QVariantMap m_databaseConfig;
        QVariantMap m_apiConfig;
};
