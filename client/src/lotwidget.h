#pragma once

#include "apiclient.h"
#include "models.h"
#include <QComboBox>
#include <QDateEdit>
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
class LotWidget;
}

class LotWidget: public QWidget {
        Q_OBJECT

    public:
        explicit LotWidget(ApiClient *apiClient, QWidget *parent = nullptr);
        ~LotWidget();

    private slots:
        void onRefreshClicked();
        void onSearchClicked();
        void onTableClicked(const QModelIndex &index);
        void onPrevPageClicked();
        void onNextPageClicked();
        void onPageSizeChanged(int index);

        // API响应处理
        void handleLotReceived(const Lot &lot);
        void handleLotListReceived(const QList<Lot> &lotList, int totalCount);
        void handleLotError(const QString &errorMessage);

    private:
        Ui::LotWidget      *ui;
        ApiClient          *m_apiClient;
        QStandardItemModel *m_model;

        int m_currentPage;
        int m_pageSize;
        int m_totalItems;

        void setupUi();
        void setupConnections();
        void setupTable();
        void updatePageInfo();
        void loadLotList();
        void updateLotDetails(const Lot &lot);
        void updateStatusDisplay(const QString &status);
};
