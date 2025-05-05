#include "views/DispatchView.h"
#include "models/DispatchModel.h"

#include <QDateTime>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>

DispatchView::DispatchView(DispatchModel *model, QWidget *parent)
    : QWidget(parent)
    , m_model(model)
{
    setupUi();

    // 连接模型信号
    connect(m_model, &DispatchModel::dataRefreshed, this, [this]() {
        // 数据刷新后更新UI
        m_tableView->resizeColumnsToContents();
    });
}

void DispatchView::setupUi()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // 顶部控制区域
    QHBoxLayout *controlLayout = new QHBoxLayout();

    // 过滤器组
    QGroupBox   *filterGroup  = new QGroupBox("筛选");
    QFormLayout *filterLayout = new QFormLayout(filterGroup);

    m_filterLot = new QLineEdit();
    m_filterLot->setPlaceholderText("批次ID");
    filterLayout->addRow("批次:", m_filterLot);

    m_filterEquipment = new QLineEdit();
    m_filterEquipment->setPlaceholderText("设备ID");
    filterLayout->addRow("设备:", m_filterEquipment);

    m_filterStatus = new QComboBox();
    m_filterStatus->addItem("全部状态", "");
    m_filterStatus->addItem("已排程", "SCHEDULED");
    m_filterStatus->addItem("运行中", "RUNNING");
    m_filterStatus->addItem("已完成", "COMPLETED");
    m_filterStatus->addItem("已取消", "CANCELLED");
    filterLayout->addRow("状态:", m_filterStatus);

    // 连接过滤器信号
    connect(m_filterLot, &QLineEdit::textChanged, this, &DispatchView::onFilterChanged);
    connect(m_filterEquipment, &QLineEdit::textChanged, this, &DispatchView::onFilterChanged);
    connect(m_filterStatus, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DispatchView::onFilterChanged);

    controlLayout->addWidget(filterGroup);

    // 操作按钮
    QVBoxLayout *buttonLayout = new QVBoxLayout();

    m_refreshButton = new QPushButton("刷新数据");
    connect(m_refreshButton, &QPushButton::clicked, this, &DispatchView::refreshData);
    buttonLayout->addWidget(m_refreshButton);

    m_createButton = new QPushButton("创建派工单");
    connect(m_createButton, &QPushButton::clicked, this, &DispatchView::onCreateDispatch);
    buttonLayout->addWidget(m_createButton);

    m_deleteButton = new QPushButton("删除派工单");
    m_deleteButton->setEnabled(false);    // 初始时禁用
    connect(m_deleteButton, &QPushButton::clicked, this, &DispatchView::onDeleteDispatch);
    buttonLayout->addWidget(m_deleteButton);

    buttonLayout->addStretch();

    controlLayout->addLayout(buttonLayout);
    controlLayout->setStretch(0, 3);    // 过滤器占更多空间
    controlLayout->setStretch(1, 1);

    mainLayout->addLayout(controlLayout);

    // 创建分隔器
    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    // 表格视图
    m_tableView = new QTableView();
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSortingEnabled(true);
    m_tableView->horizontalHeader()->setStretchLastSection(true);

    // 设置代理模型
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_tableView->setModel(m_proxyModel);

    // 连接选择变化信号
    connect(m_tableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &DispatchView::onSelectionChanged);

    splitter->addWidget(m_tableView);

    // 详情视图
    m_detailsWidget            = new QWidget();
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsWidget);

    QGroupBox   *detailsGroup = new QGroupBox("派工详情");
    QFormLayout *formLayout   = new QFormLayout(detailsGroup);

    QStringList detailKeys   = {"ID", "LOT_ID", "EQP_ID", "PRIORITY", "STATUS", "DISPATCH_TIME", "EXPECTED_START", "EXPECTED_END"};
    QStringList detailLabels = {"ID:", "批次:", "设备:", "优先级:", "状态:", "派工时间:", "预计开始:", "预计结束:"};

    for (int i = 0; i < detailKeys.size(); ++i) {
        QLabel *valueLabel            = new QLabel();
        m_detailLabels[detailKeys[i]] = valueLabel;
        formLayout->addRow(detailLabels[i], valueLabel);
    }

    detailsLayout->addWidget(detailsGroup);
    detailsLayout->addStretch();

    splitter->addWidget(m_detailsWidget);

    // 设置默认分割比例
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter, 1);    // 表格和详情占主要空间
}

void DispatchView::refreshData()
{
    m_model->refresh();
}

void DispatchView::onSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    m_deleteButton->setEnabled(current.isValid());

    if (current.isValid()) {
        updateDetails(current);
    }
    else {
        // 清空详情
        for (auto it = m_detailLabels.begin(); it != m_detailLabels.end(); ++it) {
            it.value()->clear();
        }
    }
}

void DispatchView::updateDetails(const QModelIndex &index)
{
    if (!index.isValid()) return;

    // 获取原始行索引
    int         sourceRow = m_proxyModel->mapToSource(index).row();
    QVariantMap rowData   = m_model->getRowData(sourceRow);

    // 更新详情标签
    for (auto it = m_detailLabels.begin(); it != m_detailLabels.end(); ++it) {
        QString key   = it.key();
        QLabel *label = it.value();

        if (rowData.contains(key)) {
            QVariant value = rowData.value(key);

            // 时间戳格式化
            if (key.contains("TIME") && value.toString().contains("T")) {
                QDateTime dateTime = QDateTime::fromString(value.toString(), Qt::ISODate);
                label->setText(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            }
            else {
                label->setText(value.toString());
            }

            // 为不同状态设置不同颜色
            if (key == "STATUS") {
                QString status = value.toString();
                if (status == "SCHEDULED") {
                    label->setStyleSheet("color: blue;");
                }
                else if (status == "RUNNING") {
                    label->setStyleSheet("color: green; font-weight: bold;");
                }
                else if (status == "COMPLETED") {
                    label->setStyleSheet("color: darkgreen;");
                }
                else if (status == "CANCELLED") {
                    label->setStyleSheet("color: red;");
                }
                else {
                    label->setStyleSheet("");
                }
            }

            // 为优先级设置不同颜色
            if (key == "PRIORITY") {
                int priority = value.toInt();
                if (priority <= 2) {
                    label->setStyleSheet("color: red; font-weight: bold;");
                }
                else if (priority == 3) {
                    label->setStyleSheet("color: orange;");
                }
                else {
                    label->setStyleSheet("color: green;");
                }
            }
        }
        else {
            label->setText("");
        }
    }
}

void DispatchView::onFilterChanged()
{
    QString lotFilter    = m_filterLot->text().trimmed();
    QString equipFilter  = m_filterEquipment->text().trimmed();
    QString statusFilter = m_filterStatus->currentData().toString();

    QString filterPattern;

    if (!lotFilter.isEmpty()) {
        filterPattern += lotFilter;
    }

    if (!equipFilter.isEmpty()) {
        if (!filterPattern.isEmpty()) filterPattern += "|";
        filterPattern += equipFilter;
    }

    if (!statusFilter.isEmpty()) {
        // 针对状态列特殊处理
        QModelIndex statusColIndex = m_model->index(0, 6);    // 状态列索引
        m_proxyModel->setFilterKeyColumn(6);
        m_proxyModel->setFilterFixedString(statusFilter);
    }
    else {
        // 在所有列上进行过滤
        m_proxyModel->setFilterKeyColumn(-1);
        m_proxyModel->setFilterFixedString("");
        m_proxyModel->setFilterRegularExpression(filterPattern);    // 修改这行，使用新API
    }
}

void DispatchView::onCreateDispatch()
{
    QMessageBox::information(this, "创建派工单", "创建派工单功能即将实现");
    // TODO: 实现创建派工单对话框
}

void DispatchView::onDeleteDispatch()
{
    QModelIndex current = m_tableView->currentIndex();
    if (!current.isValid()) return;

    int         sourceRow = m_proxyModel->mapToSource(current).row();
    QVariantMap rowData   = m_model->getRowData(sourceRow);

    QString lotId = rowData["LOT_ID"].toString();
    QString eqpId = rowData["EQP_ID"].toString();

    int result = QMessageBox::question(this, "确认删除", QString("确实要删除批次 %1 分配给设备 %2 的派工单吗?").arg(lotId, eqpId), QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        QMessageBox::information(this, "删除派工单", "删除派工单功能即将实现");
        // TODO: 实现删除派工单API调用
    }
}