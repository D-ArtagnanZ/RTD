#include "models/EquipmentModel.h"
#include "ApiClient.h"

EquipmentModel::EquipmentModel(ApiClient *apiClient, QObject *parent)
    : GenericTableModel(parent)
    , m_apiClient(apiClient)
{
    // 设置默认列头
    setHeaders({"设备ID", "状态", "温度(°C)", "芯片容量", "工装尺寸", "维护时间"});

    // 设置列映射
    setColumnMapping(0, "EQP_ID");
    setColumnMapping(1, "STATUS");
    setColumnMapping(2, "TEMPERATURE");
    setColumnMapping(3, "BALL_COUNT");
    setColumnMapping(4, "KIT_SIZE");
    setColumnMapping(5, "MAINTENANCE_TIME");
}

void EquipmentModel::refresh()
{
    emit refreshStarted();

    m_apiClient->getEquipment([this](bool success, const QVector<QVariantMap> &equipment) {
        if (success) {
            setData(equipment);
            emit refreshFinished(true);
        }
        else {
            emit refreshFinished(false);
        }
    });
}
