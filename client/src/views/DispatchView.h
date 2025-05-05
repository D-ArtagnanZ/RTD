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

class DispatchModel;

/**
 * @brief 派工信息视图类
 */
class DispatchView: public QWidget {
        Q_OBJECT

    public:
        explicit DispatchView(DispatchModel *model, QWidget *parent = nullptr);

    public slots:
        void refreshData();

    private slots:
        void onSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
        void onFilterChanged();
        void onCreateDispatch();
        void onDeleteDispatch();

    private:
        void setupUi();
        void updateDetails(const QModelIndex &index);

        DispatchModel         *m_model;
        QSortFilterProxyModel *m_proxyModel;

        // UI组件
        QTableView *m_tableView;
        QWidget    *m_detailsWidget;

        // 过滤控件
        QLineEdit *m_filterLot;
        QLineEdit *m_filterEquipment;
        QComboBox *m_filterStatus;

        // 操作按钮
        QPushButton *m_refreshButton;
        QPushButton *m_createButton;
        QPushButton *m_deleteButton;

        // 详情标签
        QMap<QString, QLabel *> m_detailLabels;
};
