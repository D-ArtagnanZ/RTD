#include "db/DatabaseManager.h"

#include <QDebug>
#include <QJsonDocument>
#include <stdexcept>

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    closeAllConnections();
}

void DatabaseManager::initialize(const QVariantMap &config)
{
    // 清理现有连接池
    closeAllConnections();

    // 获取默认连接池名称
    m_defaultPool = config.value("default").toString();

    // 创建连接池
    const QVariantMap connections = config.value("connections").toMap();
    for (auto it = connections.constBegin(); it != connections.constEnd(); ++it) {
        const QString     &name       = it.key();
        const QVariantMap &poolConfig = it.value().toMap();

        try {
            // 创建并存储连接池
            auto pool     = std::make_unique<ConnectionPool>(name, poolConfig, this);
            m_pools[name] = std::move(pool);

            qDebug() << "创建连接池:" << name;
        }
        catch (const std::exception &e) {
            qWarning() << "创建连接池" << name << "失败:" << e.what();
        }
    }

    if (m_pools.empty()) {
        qWarning() << "没有成功创建任何数据库连接池";
    }
    else {
        qDebug() << "成功创建" << m_pools.size() << "个数据库连接池";
    }

    // 如果默认池不存在，使用第一个池作为默认
    if (!m_defaultPool.isEmpty() && m_pools.find(m_defaultPool) == m_pools.end()) {
        if (!m_pools.empty()) {
            m_defaultPool = m_pools.begin()->first;
            qWarning() << "指定的默认连接池不存在，使用" << m_defaultPool << "作为默认连接池";
        }
        else {
            m_defaultPool.clear();
        }
    }
}

ConnectionPool *DatabaseManager::getPool(const QString &name)
{
    auto it = m_pools.find(name);
    if (it != m_pools.end()) {
        return it->second.get();
    }

    throw std::runtime_error("连接池不存在: " + name.toStdString());
}

ConnectionPool *DatabaseManager::getDefaultPool()
{
    if (m_defaultPool.isEmpty()) {
        throw std::runtime_error("未设置默认连接池");
    }

    return getPool(m_defaultPool);
}

void DatabaseManager::closeAllConnections()
{
    qDebug() << "关闭所有数据库连接";

    for (const auto &pair: m_pools) {
        pair.second->closeAllConnections();
    }

    m_pools.clear();
}

QVariantMap DatabaseManager::getPoolsStats() const
{
    QVariantList poolsStats;

    for (const auto &pair: m_pools) {
        poolsStats.append(pair.second->getStats());
    }

    QVariantMap result;
    result["default_pool"] = m_defaultPool;
    result["pools"]        = poolsStats;

    return result;
}
