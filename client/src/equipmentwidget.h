#pragma once

#include "apiclient.h"
#include "models.h"
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

namespace Ui {
class EquipmentWidget;
}

class EquipmentWidget: public QWidget {
        Q_OBJECT

    public:
        explicit EquipmentWidget(ApiClient *apiClient, QWidget *parent = nullptr);
        ~EquipmentWidget();

    private slots:
        void onRefreshClicked();
        void onSearchClicked();
        void onTableClicked(const QModelIndex &index);
        void onPrevPageClicked();
        void onNextPageClicked();
        void onPageSizeChanged(int index);

        // API响应处理
        void handleEquipmentReceived(const Equipment &equipment);
        void handleEquipmentListReceived(const QList<Equipment> &equipmentList, int totalCount);
        void handleEquipmentError(const QString &errorMessage);

    private:
        Ui::EquipmentWidget *ui;
        ApiClient           *m_apiClient;
        QStandardItemModel  *m_model;

        int m_currentPage;
        int m_pageSize;
        int m_totalItems;

        Equipment m_selectedEquipment;

        void setupUi();
        void setupConnections();
        void setupTable();
        void updatePageInfo();
        void loadEquipmentList();
        void updateEquipmentDetails(const Equipment &equipment);
        void updateStatusDisplay(const QString &status);
};
