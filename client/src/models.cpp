#include "models.h"

Equipment Equipment::fromJson(const QJsonObject &json)
{
    Equipment equipment;

    // 动态添加所有字段
    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        equipment.setField(it.key(), it.value().toVariant());
    }

    return equipment;
}

Lot Lot::fromJson(const QJsonObject &json)
{
    Lot lot;

    // 动态添加所有字段
    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        lot.setField(it.key(), it.value().toVariant());
    }

    return lot;
}

Dispatch Dispatch::fromJson(const QJsonObject &json)
{
    Dispatch dispatch;

    // 动态添加所有字段
    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        dispatch.setField(it.key(), it.value().toVariant());
    }

    return dispatch;
}
