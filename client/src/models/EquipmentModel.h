#pragma once

#include "models/GenericTableModel.h"

class ApiClient;

/**
 * @brief 机台设备模型类
 */
class EquipmentModel: public GenericTableModel {
        Q_OBJECT

    public:
        explicit EquipmentModel(ApiClient *apiClient, QObject *parent = nullptr);

        // 重写刷新方法，从API获取数据
        void refresh() override;

    private:
        ApiClient *m_apiClient;
};
