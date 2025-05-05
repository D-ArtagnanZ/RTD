#include "db/QueryExecutor.h"
#include "db/DatabaseManager.h"

#include <QDebug>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlRecord>
#include <stdexcept>

QueryExecutor::QueryExecutor(QObject *parent)
    : QObject(parent)
{
}

QVariantList QueryExecutor::executeQuery(const QString &connectionName, const QString &sql, const QVariantMap &params)
{
    try {
        // 获取连接池
        ConnectionPool *pool = DatabaseManager::instance().getPool(connectionName);

        // 获取连接
        QSqlDatabase db = pool->getConnection();

        // 准备查询
        QSqlQuery query = prepareQuery(db, sql, params);

        // 执行查询
        if (!query.exec()) {
            QString error = query.lastError().text();
            pool->releaseConnection(db);
            throw std::runtime_error("SQL执行错误: " + error.toStdString());
        }

        // 处理结果
        QVariantList results = sqlResultToVariantList(query);

        // 释放连接
        pool->releaseConnection(db);

        return results;
    }
    catch (const std::exception &e) {
        qWarning() << "查询执行失败:" << e.what() << "SQL:" << sql;
        throw;
    }
}

QVariantMap QueryExecutor::executeSingleRowQuery(const QString &connectionName, const QString &sql, const QVariantMap &params)
{
    QVariantList results = executeQuery(connectionName, sql, params);

    if (results.isEmpty()) {
        return QVariantMap();
    }

    return results.first().toMap();
}

int QueryExecutor::executeUpdate(const QString &connectionName, const QString &sql, const QVariantMap &params)
{
    try {
        // 获取连接池
        ConnectionPool *pool = DatabaseManager::instance().getPool(connectionName);

        // 获取连接
        QSqlDatabase db = pool->getConnection();

        // 准备查询
        QSqlQuery query = prepareQuery(db, sql, params);

        // 执行更新
        if (!query.exec()) {
            QString error = query.lastError().text();
            pool->releaseConnection(db);
            throw std::runtime_error("SQL执行错误: " + error.toStdString());
        }

        // 获取受影响行数
        int affectedRows = query.numRowsAffected();

        // 释放连接
        pool->releaseConnection(db);

        return affectedRows;
    }
    catch (const std::exception &e) {
        qWarning() << "更新执行失败:" << e.what() << "SQL:" << sql;
        throw;
    }
}

int QueryExecutor::executeInsert(const QString &connectionName, const QString &sql, const QVariantMap &params)
{
    try {
        // 获取连接池
        ConnectionPool *pool = DatabaseManager::instance().getPool(connectionName);

        // 获取连接
        QSqlDatabase db = pool->getConnection();

        // 准备查询
        QSqlQuery query = prepareQuery(db, sql, params);

        // 执行插入
        if (!query.exec()) {
            QString error = query.lastError().text();
            pool->releaseConnection(db);
            throw std::runtime_error("SQL执行错误: " + error.toStdString());
        }

        // 获取自增ID
        int id = -1;

        // 不同数据库获取自增ID的方式不同
        if (sql.contains("RETURNING", Qt::CaseInsensitive) && query.next()) {
            // PostgreSQL风格 - 使用RETURNING子句
            id = query.value(0).toInt();
        }
        else {
            // MySQL/SQLite风格 - 使用lastInsertId
            id = query.lastInsertId().toInt();
        }

        // 释放连接
        pool->releaseConnection(db);

        return id;
    }
    catch (const std::exception &e) {
        qWarning() << "插入执行失败:" << e.what() << "SQL:" << sql;
        throw;
    }
}

QSqlQuery QueryExecutor::prepareQuery(QSqlDatabase &db, const QString &sql, const QVariantMap &params)
{
    QSqlQuery query(db);

    // 准备查询
    if (!query.prepare(sql)) {
        QString error = query.lastError().text();
        throw std::runtime_error("SQL准备错误: " + error.toStdString());
    }

    // 绑定参数
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    return query;
}

QVariantList QueryExecutor::sqlResultToVariantList(QSqlQuery &query)
{
    QVariantList result;

    while (query.next()) {
        result.append(sqlRowToVariantMap(query));
    }

    return result;
}

QVariantMap QueryExecutor::sqlRowToVariantMap(const QSqlQuery &query)
{
    QVariantMap row;
    QSqlRecord  record = query.record();

    for (int i = 0; i < record.count(); i++) {
        QString  fieldName = record.fieldName(i);
        QVariant value     = query.value(i);

        row[fieldName] = value;
    }

    return row;
}
