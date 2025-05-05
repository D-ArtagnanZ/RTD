#pragma once

#include "models/GenericTableModel.h"

class ApiClient;

/**
 * @brief 产品批次模型类
 */
class LotModel: public GenericTableModel {
        Q_OBJECT

    public:
        explicit LotModel(ApiClient *apiClient, QObject *parent = nullptr);

        // 重写刷新方法，从API获取数据
        void refresh() override;

    private:
        ApiClient *m_apiClient;
};
