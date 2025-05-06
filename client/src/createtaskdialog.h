#pragma once

#include "apiclient.h"
#include "models.h"
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QMessageBox>

namespace Ui {
class CreateTaskDialog;
}

class CreateTaskDialog: public QDialog {
        Q_OBJECT

    public:
        explicit CreateTaskDialog(ApiClient *apiClient, QWidget *parent = nullptr);
        ~CreateTaskDialog();

    signals:
        void taskCreated(const Dispatch &dispatch);

    private slots:
        void onCreateButtonClicked();
        void onCancelButtonClicked();

        // 设备批次数据加载
        void handleEquipmentListReceived(const QList<Equipment> &equipmentList, int totalCount);
        void handleLotListReceived(const QList<Lot> &lotList, int totalCount);
        void handleEquipmentError(const QString &errorMessage);
        void handleLotError(const QString &errorMessage);

    private:
        Ui::CreateTaskDialog *ui;
        ApiClient            *m_apiClient;

        // 设备和批次信息缓存
        QList<Equipment> m_equipmentList;
        QList<Lot>       m_lotList;

        void setupUi();
        void setupConnections();
        void loadEquipmentData();
        void loadLotData();
};
