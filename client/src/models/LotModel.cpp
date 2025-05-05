#include "models/LotModel.h"
#include "ApiClient.h"

LotModel::LotModel(ApiClient *apiClient, QObject *parent)
    : GenericTableModel(parent)
    , m_apiClient(apiClient)
{
    // 设置默认列头
    setHeaders({"批次ID", "状态", "产品ID", "流程", "工序", "芯片数量", "创建时间", "更新时间"});

    // 设置列映射（列索引到数据属性的映射）
    setColumnMapping(0, "LOT_ID");
    setColumnMapping(1, "STATUS");
    setColumnMapping(2, "PDID");
    setColumnMapping(3, "FLOW");
    setColumnMapping(4, "OPER");
    setColumnMapping(5, "BALL_COUNT");
    setColumnMapping(6, "CREATION_TIME");
    setColumnMapping(7, "UPDATE_TIME");
}

void LotModel::refresh()
{
    emit refreshStarted();

    m_apiClient->getLots([this](bool success, const QVector<QVariantMap> &lots) {
        if (success) {
            setData(lots);
            emit refreshFinished(true);
        }
        else {
            emit refreshFinished(false);
        }
    });
}
