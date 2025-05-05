#include "views/LotListView.h"
#include "models/LotModel.h"

#include <QDateTime>
#include <QFormLayout>
#include <QHeaderView>

LotListView::LotListView(LotModel *model, QWidget *parent)
    : QWidget(parent)
    , m_model(model)
{
    setupUi();

    // 连接模型信号
    connect(m_model, &LotModel::dataRefreshed, this, [this]() {
        // 数据刷新后更新UI
        m_tableView->resizeColumnsToContents();
    });
}

void LotListView::setupUi()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // 顶部控制区域
    QHBoxLayout *controlLayout = new QHBoxLayout();

    // 过滤器组
    QGroupBox   *filterGroup  = new QGroupBox("筛选产品批次");
    QFormLayout *filterLayout = new QFormLayout(filterGroup);

    m_filterLotId = new QLineEdit();
    m_filterLotId->setPlaceholderText("批次ID");
    filterLayout->addRow("批次ID:", m_filterLotId);

    m_filterPdid = new QLineEdit();
    m_filterPdid->setPlaceholderText("产品ID");
    filterLayout->addRow("产品ID:", m_filterPdid);

    m_filterStatus = new QComboBox();
    m_filterStatus->addItem("全部状态", "");
    m_filterStatus->addItem("等待中", "WAITING");
    m_filterStatus->addItem("加工中", "PROCESSING");
    m_filterStatus->addItem("已完成", "COMPLETED");
    m_filterStatus->addItem("暂停", "HOLD");
    m_filterStatus->addItem("终止", "ABORTED");
    filterLayout->addRow("状态:", m_filterStatus);

    // 连接过滤器信号
    connect(m_filterLotId, &QLineEdit::textChanged, this, &LotListView::onFilterChanged);
    connect(m_filterPdid, &QLineEdit::textChanged, this, &LotListView::onFilterChanged);
    connect(m_filterStatus, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LotListView::onFilterChanged);

    controlLayout->addWidget(filterGroup);

    // 操作按钮
    QVBoxLayout *buttonLayout = new QVBoxLayout();

    m_refreshButton = new QPushButton("刷新数据");
    connect(m_refreshButton, &QPushButton::clicked, this, &LotListView::refreshData);
    buttonLayout->addWidget(m_refreshButton);

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
    connect(m_tableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &LotListView::onSelectionChanged);

    splitter->addWidget(m_tableView);

    // 详情视图
    m_detailsWidget            = new QWidget();
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsWidget);

    QGroupBox   *detailsGroup = new QGroupBox("产品批次详情");
    QFormLayout *formLayout   = new QFormLayout(detailsGroup);

    QStringList detailKeys   = {"LOT_ID", "STATUS", "PDID", "FLOW", "OPER", "BALL_COUNT", "CREATION_TIME", "UPDATE_TIME"};
    QStringList detailLabels = {"批次ID:", "状态:", "产品ID:", "流程:", "工序:", "芯片数:", "创建时间:", "更新时间:"};

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

void LotListView::refreshData()
{
    m_model->refresh();
}

void LotListView::onSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
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

void LotListView::updateDetails(const QModelIndex &index)
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
                if (status == "WAITING") {
                    label->setStyleSheet("color: blue;");
                }
                else if (status == "PROCESSING") {
                    label->setStyleSheet("color: orange; font-weight: bold;");
                }
                else if (status == "COMPLETED") {
                    label->setStyleSheet("color: green;");
                }
                else if (status == "HOLD") {
                    label->setStyleSheet("color: red;");
                }
                else if (status == "ABORTED") {
                    label->setStyleSheet("color: darkred;");
                }
                else {
                    label->setStyleSheet("");
                }
            }
        }
        else {
            label->setText("");
        }
    }
}

void LotListView::onFilterChanged()
{
    QString lotIdFilter  = m_filterLotId->text().trimmed();
    QString pdidFilter   = m_filterPdid->text().trimmed();
    QString statusFilter = m_filterStatus->currentData().toString();

    QString filterPattern;

    if (!lotIdFilter.isEmpty()) {
        filterPattern += lotIdFilter;
    }

    if (!pdidFilter.isEmpty()) {
        if (!filterPattern.isEmpty()) filterPattern += "|";
        filterPattern += pdidFilter;
    }

    if (!statusFilter.isEmpty()) {
        // 针对状态列特殊处理
        m_proxyModel->setFilterKeyColumn(1);    // 状态列索引
        m_proxyModel->setFilterFixedString(statusFilter);
    }
    else {
        // 在所有列上进行过滤
        m_proxyModel->setFilterKeyColumn(-1);
        m_proxyModel->setFilterFixedString("");
        m_proxyModel->setFilterRegularExpression(filterPattern);
    }
}
