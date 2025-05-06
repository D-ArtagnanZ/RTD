#pragma once

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>

// 基础数据模型类
class BaseModel {
    public:
        // 存储所有字段的映射
        QVariantMap fields;

        // 设置字段值
        void setField(const QString &name, const QVariant &value)
        {
            fields[name] = value;
        }

        // 获取字段值
        QVariant field(const QString &name, const QVariant &defaultValue = QVariant()) const
        {
            return fields.value(name, defaultValue);
        }

        // 判断是否包含某个字段
        bool hasField(const QString &name) const
        {
            return fields.contains(name);
        }

        // 获取所有字段名
        QStringList fieldNames() const
        {
            return fields.keys();
        }
};

// 设备数据模型
class Equipment: public BaseModel {
    public:
        // 便捷访问器
        QString id() const { return field("ID").toString(); }
        QString name() const { return field("NAME").toString(); }
        QString type() const { return field("TYPE").toString(); }
        QString status() const { return field("STATUS").toString(); }
        QString location() const { return field("LOCATION").toString(); }
        QString description() const { return field("DESCRIPTION").toString(); }

        static Equipment fromJson(const QJsonObject &json);
};

// 批次数据模型
class Lot: public BaseModel {
    public:
        // 便捷访问器
        QString id() const { return field("ID").toString(); }
        QString name() const { return field("NAME").toString(); }
        QString product() const { return field("PRODUCT").toString(); }
        QString status() const { return field("STATUS").toString(); }
        int     quantity() const { return field("QUANTITY").toInt(); }
        QString priority() const { return field("PRIORITY").toString(); }
        QString startDate() const { return field("START_DATE").toString(); }
        QString dueDate() const { return field("DUE_DATE").toString(); }

        static Lot fromJson(const QJsonObject &json);
};

// 调度数据模型
class Dispatch: public BaseModel {
    public:
        // 便捷访问器
        QString id() const { return field("ID").toString(); }
        QString eqpId() const { return field("EQP_ID").toString(); }
        QString lotId() const { return field("LOT_ID").toString(); }
        QString status() const { return field("STATUS").toString(); }
        QString startTime() const { return field("START_TIME").toString(); }
        QString endTime() const { return field("END_TIME").toString(); }
        QString operatorId() const { return field("OPERATOR_ID").toString(); }
        QString priority() const { return field("PRIORITY").toString(); }

        static Dispatch fromJson(const QJsonObject &json);
};

// 分页结果数据模型
template<typename T>
class PagedResult {
    public:
        QList<T> items;
        int      currentPage;
        int      pageSize;
        int      totalItems;
        int      totalPages;

        static PagedResult<T> fromJson(const QJsonObject                    &json,
                                       std::function<T(const QJsonObject &)> converter);
};

template<typename T>
PagedResult<T> PagedResult<T>::fromJson(const QJsonObject                    &json,
                                        std::function<T(const QJsonObject &)> converter)
{
    PagedResult<T> result;
    if (json.contains("data") && json["data"].isArray()) {
        QJsonArray dataArray = json["data"].toArray();
        for (const QJsonValue &value: dataArray) {
            if (value.isObject()) {
                T item = converter(value.toObject());
                result.items.append(item);
            }
        }
    }

    // 在实际应用中，服务器响应应该包含分页信息
    // 这里假设服务器响应包含以下字段或者从页面大小和当前页码计算
    result.currentPage = 1;    // 默认值，实际应从服务器响应获取
    result.pageSize    = result.items.size();
    result.totalItems  = result.items.size();    // 默认值，实际应从服务器响应获取
    result.totalPages  = 1;                      // 默认值，实际应从服务器响应获取

    return result;
}
