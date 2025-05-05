#include "db/ConnectionPool.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QThread>

ConnectionPool::ConnectionPool(const QString &poolName, const QVariantMap &config, QObject *parent)
    : QObject(parent)
    , m_poolName(poolName)
    , m_dbType(config.value("type").toString())
    , m_connectionParams(config)
    , m_maxConnections(config.value("pool").toMap().value("max_size", 10).toInt())
    , m_idleTimeout(config.value("pool").toMap().value("idle_timeout", 300).toInt())              // 默认5分钟
    , m_connectionTimeout(config.value("pool").toMap().value("connection_timeout", 5).toInt())    // 默认5秒
{
    qDebug() << "初始化连接池:" << m_poolName << "类型:" << m_dbType;

    // 设置清理空闲连接的定时器
    connect(&m_cleanupTimer, &QTimer::timeout, this, &ConnectionPool::cleanIdleConnections);
    m_cleanupTimer.start(60000);    // 每分钟检查一次
}

ConnectionPool::~ConnectionPool()
{
    closeAllConnections();
}

QSqlDatabase ConnectionPool::getConnection()
{
    QMutexLocker locker(&m_mutex);

    // 寻找可用连接
    for (auto &conn: m_connections) {
        if (!conn.inUse) {
            QSqlDatabase db = QSqlDatabase::database(conn.connectionName);

            // 检查连接是否有效
            if (db.isValid() && db.isOpen()) {
                conn.inUse    = true;
                conn.lastUsed = QDateTime::currentDateTime();
                return db;
            }

            // 连接无效，尝试重新打开
            if (db.open()) {
                conn.inUse    = true;
                conn.lastUsed = QDateTime::currentDateTime();
                return db;
            }

            // 如果无法重新打开，我们将在下面创建一个新的连接
            qWarning() << "无法重新打开数据库连接:" << db.lastError().text();
        }
    }

    // 如果没有可用连接且未达到最大连接数，则创建新连接
    if (m_connections.size() < m_maxConnections) {
        QSqlDatabase db = createConnection();
        if (db.isValid() && db.isOpen()) {
            PooledConnection newConn;
            newConn.connectionName = db.connectionName();
            newConn.lastUsed       = QDateTime::currentDateTime();
            newConn.inUse          = true;
            m_connections.append(newConn);
            return db;
        }
    }

    // 无法创建新连接，尝试等待一个可用连接
    qWarning() << "连接池" << m_poolName << "已满，等待可用连接...";

    // 释放锁，等待一会儿再试
    locker.unlock();
    QThread::msleep(100);    // 短暂等待

    // 递归调用，最多等待connectionTimeout秒
    static thread_local int retryCount = 0;
    if (++retryCount < m_connectionTimeout * 10) {    // 每秒尝试10次
        QSqlDatabase db = getConnection();
        retryCount      = 0;    // 重置重试计数
        return db;
    }

    retryCount = 0;    // 重置重试计数
    throw std::runtime_error("无法获取数据库连接，连接池已满且超时");
}

void ConnectionPool::releaseConnection(const QSqlDatabase &connection)
{
    QMutexLocker locker(&m_mutex);

    QString connName = connection.connectionName();
    for (auto &conn: m_connections) {
        if (conn.connectionName == connName) {
            conn.inUse    = false;
            conn.lastUsed = QDateTime::currentDateTime();
            return;
        }
    }
}

void ConnectionPool::closeAllConnections()
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "关闭连接池" << m_poolName << "的所有连接";

    for (const auto &conn: m_connections) {
        {
            QSqlDatabase db = QSqlDatabase::database(conn.connectionName);
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(conn.connectionName);
    }

    m_connections.clear();
}

QVariantMap ConnectionPool::getStats() const
{
    QMutexLocker locker(&m_mutex);

    int activeCount = 0;
    int idleCount   = 0;

    for (const auto &conn: m_connections) {
        if (conn.inUse) {
            activeCount++;
        }
        else {
            idleCount++;
        }
    }

    QVariantMap stats;
    stats["poolName"]          = m_poolName;
    stats["type"]              = m_dbType;
    stats["maxConnections"]    = m_maxConnections;
    stats["activeConnections"] = activeCount;
    stats["idleConnections"]   = idleCount;
    stats["totalConnections"]  = m_connections.size();

    return stats;
}

void ConnectionPool::cleanIdleConnections()
{
    QMutexLocker locker(&m_mutex);

    QDateTime                              now = QDateTime::currentDateTime();
    QMutableListIterator<PooledConnection> i(m_connections);

    while (i.hasNext()) {
        auto &conn = i.next();

        // 只检查空闲连接
        if (!conn.inUse) {
            // 如果空闲时间超过阈值且不是唯一的连接，则关闭
            if (conn.lastUsed.secsTo(now) > m_idleTimeout && m_connections.size() > 1) {
                QSqlDatabase db = QSqlDatabase::database(conn.connectionName);
                if (db.isOpen()) {
                    db.close();
                }
                QSqlDatabase::removeDatabase(conn.connectionName);
                i.remove();
            }
        }
    }
}

QSqlDatabase ConnectionPool::createConnection()
{
    QString connectionName = QString("%1_%2_%3")
                               .arg(m_poolName)
                               .arg(quintptr(QThread::currentThreadId()), 0, 16)
                               .arg(++m_nextConnectionId);

    QSqlDatabase db = QSqlDatabase::addDatabase(m_dbType, connectionName);

    if (m_dbType == "QPSQL") {
        db.setHostName(m_connectionParams.value("host").toString());
        db.setPort(m_connectionParams.value("port").toInt());
        db.setDatabaseName(m_connectionParams.value("database").toString());
        db.setUserName(m_connectionParams.value("username").toString());
        db.setPassword(m_connectionParams.value("password").toString());

        // 设置其他PostgreSQL特定选项
        QVariantMap options = m_connectionParams.value("options").toMap();
        QString optionsString;
        for (auto it = options.constBegin(); it != options.constEnd(); ++it) {
            if (!optionsString.isEmpty()) {
                optionsString += ";";
            }
            optionsString += it.key() + "=" + it.value().toString();
        }
        if (!optionsString.isEmpty()) {
            db.setConnectOptions(optionsString);
        }
    }
    else if (m_dbType == "QOCI") {
        db.setDatabaseName(m_connectionParams.value("connection_string").toString());
        db.setUserName(m_connectionParams.value("username").toString());
        db.setPassword(m_connectionParams.value("password").toString());
    }
    else if (m_dbType == "QSQLITE") {
        db.setDatabaseName(m_connectionParams.value("database").toString());
    }
    else {
        qWarning() << "不支持的数据库类型:" << m_dbType;
        throw std::runtime_error("不支持的数据库类型: " + m_dbType.toStdString());
    }

    // 尝试打开连接
    if (!db.open()) {
        qWarning() << "无法创建数据库连接:" << db.lastError().text();
        QSqlDatabase::removeDatabase(connectionName);
        throw std::runtime_error("连接数据库失败: " + db.lastError().text().toStdString());
    }

    return db;
}
