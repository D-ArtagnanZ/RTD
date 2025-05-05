#pragma once

#include "db/ConnectionPool.h"

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QVariantMap>
#include <memory>
#include <unordered_map>

class DatabaseManager: public QObject {
        Q_OBJECT

    public:
        static DatabaseManager &instance();

        // 初始化数据库连接池
        void initialize(const QVariantMap &config);

        // 获取连接池
        ConnectionPool *getPool(const QString &name);

        // 获取默认连接池
        ConnectionPool *getDefaultPool();

        // 关闭所有连接
        void closeAllConnections();

        // 获取所有连接池状态
        QVariantMap getPoolsStats() const;

    private:
        DatabaseManager(QObject *parent = nullptr);
        ~DatabaseManager();

        DatabaseManager(const DatabaseManager &)            = delete;
        DatabaseManager &operator=(const DatabaseManager &) = delete;

        std::unordered_map<QString, std::unique_ptr<ConnectionPool>> m_pools;
        QString                                                      m_defaultPool;
};
