#pragma once

#include <QDateTime>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QSqlDatabase>
#include <QTimer>
#include <QVariantMap>
#include <memory>

class ConnectionPool: public QObject {
        Q_OBJECT

    public:
        explicit ConnectionPool(const QString &poolName, const QVariantMap &config, QObject *parent = nullptr);
        ~ConnectionPool() override;

        // 获取数据库连接
        QSqlDatabase getConnection();

        // 释放数据库连接
        void releaseConnection(const QSqlDatabase &connection);

        // 关闭所有连接
        void closeAllConnections();

        // 获取连接池名称
        QString poolName() const { return m_poolName; }

        // 获取连接池状态信息
        QVariantMap getStats() const;

    private slots:
        void cleanIdleConnections();

    private:
        // 创建新连接
        QSqlDatabase createConnection();

        QString     m_poolName;             // 连接池名称
        QString     m_dbType;               // 数据库类型
        QVariantMap m_connectionParams;     // 连接参数
        int         m_maxConnections;       // 最大连接数
        int         m_idleTimeout;          // 空闲超时(秒)
        int         m_connectionTimeout;    // 连接超时(秒)

        struct PooledConnection {
                QString   connectionName;
                QDateTime lastUsed;
                bool      inUse = false;
        };

        QList<PooledConnection> m_connections;             // 连接列表
        mutable QMutex          m_mutex;                   // 互斥锁
        QTimer                  m_cleanupTimer;            // 清理定时器
        int                     m_nextConnectionId = 0;    // 连接ID计数器
};
