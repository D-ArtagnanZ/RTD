#include "views/EquipmentListView.h"
#include "models/EquipmentModel.h"

#include <QDateTime>
#include <QFormLayout>
#include <QHeaderView>

EquipmentListView::EquipmentListView(EquipmentModel *model, QWidget *parent)
    : QWidget(parent)
    , m_model(model)
{
    setupUi();

    // 连接模型信号
    connect(m_model, &EquipmentModel::dataRefreshed, this, [this]() {
        // 数据刷新后更新UI
        m_tableView->resizeColumnsToContents();
    });
}

void EquipmentListView::setupUi()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // 顶部控制区域
    QHBoxLayout *controlLayout = new QHBoxLayout();

    // 过滤器组
    QGroupBox   *filterGroup  = new QGroupBox("筛选机台");
    QFormLayout *filterLayout = new QFormLayout(filterGroup);

    m_filterEqpId = new QLineEdit();
    m_filterEqpId->setPlaceholderText("设备ID");
    filterLayout->addRow("设备ID:", m_filterEqpId);

    m_filterStatus = new QComboBox();
    m_filterStatus->addItem("全部状态", "");
    m_filterStatus->addItem("空闲", "IDLE");
    m_filterStatus->addItem("运行中", "RUNNING");
    m_filterStatus->addItem("维护中", "MAINTENANCE");
    m_filterStatus->addItem("错误", "ERROR");
    m_filterStatus->addItem("离线", "OFFLINE");
    filterLayout->addRow("状态:", m_filterStatus);

    // 连接过滤器信号
    connect(m_filterEqpId, &QLineEdit::textChanged, this, &EquipmentListView::onFilterChanged);
    connect(m_filterStatus, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EquipmentListView::onFilterChanged);

    controlLayout->addWidget(filterGroup);

    // 操作按钮
    QVBoxLayout *buttonLayout = new QVBoxLayout();

    m_refreshButton = new QPushButton("刷新数据");
    connect(m_refreshButton, &QPushButton::clicked, this, &EquipmentListView::refreshData);
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
    connect(m_tableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &EquipmentListView::onSelectionChanged);

    splitter->addWidget(m_tableView);

    // 详情视图
    m_detailsWidget            = new QWidget();
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsWidget);

    QGroupBox   *detailsGroup = new QGroupBox("设备详情");
    QFormLayout *formLayout   = new QFormLayout(detailsGroup);

    QStringList detailKeys   = {"EQP_ID", "STATUS", "TEMPERATURE", "BALL_COUNT", "KIT_SIZE", "MAINTENANCE_TIME"};
    QStringList detailLabels = {"设备ID:", "状态:", "温度:", "芯片容量:", "工装尺寸:", "维护时间:"};

    for (int i = 0; i < detailKeys.size(); ++i) {
        QLabel *valueLabel            = new QLabel();
        m_detailLabels[detailKeys[i]] = valueLabel;
        formLayout->addRow(detailLabels[i], valueLabel);
    }

    // 添加温度进度条
    QLabel *tempLabel = new QLabel("温度状态:");
    m_temperatureBar  = new QProgressBar();
    m_temperatureBar->setRange(0, 100);
    m_temperatureBar->setFormat("%v℃");
    formLayout->addRow(tempLabel, m_temperatureBar);

    detailsLayout->addWidget(detailsGroup);
    detailsLayout->addStretch();

    splitter->addWidget(m_detailsWidget);

    // 设置默认分割比例
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter, 1);    // 表格和详情占主要空间
}

void EquipmentListView::refreshData()
{
    m_model->refresh();
}

void EquipmentListView::onSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (current.isValid()) {
        updateDetails(current);
    }
    else {
        // 清空详情
        for (auto it = m_detailLabels.begin(); it != m_detailLabels.end(); ++it) {
            it.value()->clear();
        }
        m_temperatureBar->setValue(0);
    }
}

void EquipmentListView::updateDetails(const QModelIndex &index)
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

            // 特殊格式化
            if (key == "TEMPERATURE") {
                double temp = value.toDouble();
                label->setText(QString("%1 °C").arg(temp, 0, 'f', 1));

                // 更新温度进度条
                m_temperatureBar->setValue(static_cast<int>(temp));

                // 根据温度设置进度条颜色
                if (temp < 50) {
                    m_temperatureBar->setStyleSheet("QProgressBar { text-align: center; } "
                                                    "QProgressBar::chunk { background-color: green; }");
                }
                else if (temp < 75) {
                    m_temperatureBar->setStyleSheet("QProgressBar { text-align: center; } "
                                                    "QProgressBar::chunk { background-color: orange; }");
                }
                else {
                    m_temperatureBar->setStyleSheet("QProgressBar { text-align: center; } "
                                                    "QProgressBar::chunk { background-color: red; }");
                }
            }
            else if (key == "MAINTENANCE_TIME") {
                label->setText(value.toString());
            }
            else {
                label->setText(value.toString());
            }

            // 为不同状态设置不同颜色
            if (key == "STATUS") {
                QString status = value.toString();
                if (status == "IDLE") {
                    label->setStyleSheet("color: blue;");
                }
                else if (status == "RUNNING") {
                    label->setStyleSheet("color: green; font-weight: bold;");
                }
                else if (status == "MAINTENANCE") {
                    label->setStyleSheet("color: orange;");
                }
                else if (status == "ERROR") {
                    label->setStyleSheet("color: red; font-weight: bold;");
                }
                else if (status == "OFFLINE") {
                    label->setStyleSheet("color: gray;");
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

void EquipmentListView::onFilterChanged()
{
    QString eqpIdFilter  = m_filterEqpId->text().trimmed();
    QString statusFilter = m_filterStatus->currentData().toString();

    QString filterPattern;

    if (!eqpIdFilter.isEmpty()) {
        filterPattern += eqpIdFilter;
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
