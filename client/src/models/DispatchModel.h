#pragma once

#include "models/GenericTableModel.h"

class ApiClient;

/**
 * @brief 派工单模型类
 */
class DispatchModel: public GenericTableModel {
        Q_OBJECT

    public:
        explicit DispatchModel(ApiClient *apiClient, QObject *parent = nullptr);

        // 重写刷新方法，从API获取数据
        void refresh() override;

    private:
        ApiClient *m_apiClient;
};
