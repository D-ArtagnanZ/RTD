#pragma once

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

class LotModel;

class LotListView: public QWidget {
        Q_OBJECT

    public:
        explicit LotListView(LotModel *model, QWidget *parent = nullptr);

    public slots:
        void refreshData();

    private slots:
        void onSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
        void onFilterChanged();

    private:
        void setupUi();
        void updateDetails(const QModelIndex &index);

        LotModel              *m_model;
        QSortFilterProxyModel *m_proxyModel;

        // UI组件
        QTableView *m_tableView;
        QWidget    *m_detailsWidget;

        // 过滤控件
        QLineEdit *m_filterLotId;
        QLineEdit *m_filterPdid;
        QComboBox *m_filterStatus;

        // 操作按钮
        QPushButton *m_refreshButton;

        // 详情标签
        QMap<QString, QLabel *> m_detailLabels;
};
