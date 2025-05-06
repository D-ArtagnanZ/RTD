#pragma once

#include "apiclient.h"
#include "createtaskdialog.h"
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
class DispatchWidget;
}

class DispatchWidget: public QWidget {
        Q_OBJECT

    public:
        explicit DispatchWidget(ApiClient *apiClient, QWidget *parent = nullptr);
        ~DispatchWidget();

    private slots:
        void onRefreshClicked();
        void onSearchClicked();    // 使用单一搜索入口点
        void onCreateTaskClicked();
        void onTableClicked(const QModelIndex &index);
        void onPrevPageClicked();
        void onNextPageClicked();
        void onPageSizeChanged(int index);
        void onSearchModeChanged(int index);

        // API响应处理
        void handleDispatchListReceived(const QList<Dispatch> &dispatchList, int totalCount);
        void handleDispatchByEquipmentReceived(const QList<Dispatch> &dispatchList);
        void handleDispatchByLotReceived(const QList<Dispatch> &dispatchList);    // 添加批次搜索响应处理
        void handleDispatchCreated(bool success, const QString &message);
        void handleDispatchError(const QString &errorMessage);

        // 任务对话框
        void onTaskCreated(const Dispatch &dispatch);

    private:
        Ui::DispatchWidget *ui;
        ApiClient          *m_apiClient;
        QStandardItemModel *m_model;

        int m_currentPage;
        int m_pageSize;
        int m_totalItems;

        // 定义搜索模式枚举
        enum SearchMode {
            SearchByEquipment = 0,
            SearchByLot       = 1
        };

        SearchMode m_currentSearchMode;    // 当前搜索模式

        void setupUi();
        void setupConnections();
        void setupTable();
        void updatePageInfo();
        void loadDispatchList();
        void updateStatusDisplay(const QModelIndex &index, const QString &status);
};
