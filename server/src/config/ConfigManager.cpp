#include "config/ConfigManager.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

ConfigManager &ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::loadConfigs(const QString &serverConfigPath)
{
    try {
        // 加载服务器配置
        m_serverConfig = loadJsonFile(serverConfigPath);
        qDebug() << "服务器配置加载成功:" << serverConfigPath;

        // 加载数据库配置
        QString dbConfigPath = "config/database.json";
        m_databaseConfig     = loadJsonFile(dbConfigPath);
        qDebug() << "数据库配置加载成功:" << dbConfigPath;

        // 加载API配置
        QString apiConfigPath = "config/api.json";
        m_apiConfig           = loadJsonFile(apiConfigPath);
        qDebug() << "API配置加载成功:" << apiConfigPath;

        return true;
    }
    catch (const std::exception &e) {
        qCritical() << "加载配置失败:" << e.what();
        return false;
    }
}

QVariantMap ConfigManager::getServerConfig() const
{
    return m_serverConfig;
}

QVariantMap ConfigManager::getDatabaseConfig() const
{
    return m_databaseConfig;
}

QVariantMap ConfigManager::getApiConfig() const
{
    return m_apiConfig;
}

QVariant ConfigManager::getConfigValue(const QString &section, const QString &key, const QVariant &defaultValue) const
{
    QVariantMap config;

    if (section == "server") {
        config = m_serverConfig;
    }
    else if (section == "database") {
        config = m_databaseConfig;
    }
    else if (section == "api") {
        config = m_apiConfig;
    }

    // 支持嵌套键，如"server.port"
    QStringList parts = key.split('.');
    QVariant    value = config;

    for (const QString &part: parts) {
        if (!value.isValid() || value.typeId() != QMetaType::QVariantMap) {
            return defaultValue;
        }

        QVariantMap map = value.toMap();
        if (!map.contains(part)) {
            return defaultValue;
        }

        value = map.value(part);
    }

    return value.isValid() ? value : defaultValue;
}

QVariantMap ConfigManager::loadJsonFile(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("无法打开配置文件: " + path.toStdString());
    }

    QByteArray      data = file.readAll();
    QJsonParseError error;
    QJsonDocument   doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        throw std::runtime_error("解析配置文件失败: " + error.errorString().toStdString());
    }

    return doc.toVariant().toMap();
}
