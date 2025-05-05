#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

class ConnectionPool;

class QueryExecutor: public QObject {
        Q_OBJECT

    public:
        explicit QueryExecutor(QObject *parent = nullptr);

        // 简单查询方法
        QVariantList executeQuery(const QString &connectionName, const QString &sql, const QVariantMap &params = QVariantMap());

        // 执行单行返回查询
        QVariantMap executeSingleRowQuery(const QString &connectionName, const QString &sql, const QVariantMap &params = QVariantMap());

        // 执行更新语句
        int executeUpdate(const QString &connectionName, const QString &sql, const QVariantMap &params = QVariantMap());

        // 执行插入并返回自增ID
        int executeInsert(const QString &connectionName, const QString &sql, const QVariantMap &params = QVariantMap());

    private:
        // 准备并绑定参数
        QSqlQuery prepareQuery(QSqlDatabase &db, const QString &sql, const QVariantMap &params);

        // 将结果转换为变体列表
        QVariantList sqlResultToVariantList(QSqlQuery &query);

        // 将单行结果转换为变体映射
        QVariantMap sqlRowToVariantMap(const QSqlQuery &query);
};
