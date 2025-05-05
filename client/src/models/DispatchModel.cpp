#include "models/DispatchModel.h"
#include "ApiClient.h"

DispatchModel::DispatchModel(ApiClient* apiClient, QObject* parent)
    : GenericTableModel(parent)
    , m_apiClient(apiClient)
{
    // 设置默认列头
    setHeaders({"批次ID", "设备ID", "优先级", "派工时间", "预计开始", "预计结束", "状态"});
    
    // 设置列映射
    setColumnMapping(0, "LOT_ID");
    setColumnMapping(1, "EQP_ID");
    setColumnMapping(2, "PRIORITY");
    setColumnMapping(3, "DISPATCH_TIME");
    setColumnMapping(4, "EXPECTED_START");
    setColumnMapping(5, "EXPECTED_END");
    setColumnMapping(6, "STATUS");
}

void DispatchModel::refresh()
{
    emit refreshStarted();
    
    m_apiClient->getDispatches([this](bool success, const QVector<QVariantMap>& dispatches) {
        if (success) {
            setData(dispatches);
            emit refreshFinished(true);
        } else {
            emit refreshFinished(false);
        }
    });
}
