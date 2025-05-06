#include "dispatchwidget.h"
#include "ui_dispatchwidget.h"

DispatchWidget::DispatchWidget(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), ui(new Ui::DispatchWidget), m_apiClient(apiClient), m_currentPage(1), m_pageSize(10), m_totalItems(0), m_currentSearchMode(SearchMode::SearchByEquipment)    // 默认按设备ID搜索
{
    ui->setupUi(this);

    // 设置表格模型
    m_model = new QStandardItemModel(this);

    setupUi();
    setupConnections();
    setupTable();

    // 初始加载调度列表
    loadDispatchList();
}

DispatchWidget::~DispatchWidget()
{
    delete ui;
}

void DispatchWidget::setupUi()
{
    // 设置页面大小下拉框
    ui->pageSizeCombo->addItem("10条/页", 10);
    ui->pageSizeCombo->addItem("20条/页", 20);
    ui->pageSizeCombo->addItem("50条/页", 50);
    ui->pageSizeCombo->addItem("100条/页", 100);

    // 设置表格
    ui->tableView->setModel(m_model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setAlternatingRowColors(true);

    // 设置按钮图标
    ui->refreshButton->setIcon(QIcon(":/images/refresh.png"));
    ui->searchButton->setIcon(QIcon(":/images/search.png"));
    ui->createTaskButton->setIcon(QIcon(":/images/add.png"));

    // 设置按钮样式
    ui->refreshButton->setProperty("class", "actionButton");
    ui->searchButton->setProperty("class", "actionButton");
    ui->createTaskButton->setProperty("class", "actionButton");
    ui->prevButton->setProperty("class", "pageButton");
    ui->nextButton->setProperty("class", "pageButton");

    // 设置搜索模式选择
    ui->searchModeCombo->addItem("按设备ID搜索", SearchMode::SearchByEquipment);
    ui->searchModeCombo->addItem("按批次ID搜索", SearchMode::SearchByLot);

    // 设置搜索框样式
    ui->searchEdit->setProperty("class", "searchInput");
    ui->searchEdit->setPlaceholderText("输入设备ID搜索调度");
}

void DispatchWidget::setupConnections()
{
    // 按钮连接
    connect(ui->refreshButton, &QPushButton::clicked, this, &DispatchWidget::onRefreshClicked);
    connect(ui->searchButton, &QPushButton::clicked, this, &DispatchWidget::onSearchClicked);
    connect(ui->createTaskButton, &QPushButton::clicked, this, &DispatchWidget::onCreateTaskClicked);
    connect(ui->prevButton, &QPushButton::clicked, this, &DispatchWidget::onPrevPageClicked);
    connect(ui->nextButton, &QPushButton::clicked, this, &DispatchWidget::onNextPageClicked);

    // 表格选择连接
    connect(ui->tableView, &QTableView::clicked, this, &DispatchWidget::onTableClicked);

    // 页面大小连接
    connect(ui->pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DispatchWidget::onPageSizeChanged);

    // 搜索相关连接
    connect(ui->searchButton, &QPushButton::clicked, this, &DispatchWidget::onSearchClicked);
    connect(ui->searchModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DispatchWidget::onSearchModeChanged);

    // API客户端连接
    connect(m_apiClient, &ApiClient::dispatchListReceived, this, &DispatchWidget::handleDispatchListReceived);
    connect(m_apiClient, &ApiClient::dispatchByEquipmentReceived, this, &DispatchWidget::handleDispatchByEquipmentReceived);
    connect(m_apiClient, &ApiClient::dispatchByLotReceived, this, &DispatchWidget::handleDispatchByLotReceived);
    connect(m_apiClient, &ApiClient::dispatchCreated, this, &DispatchWidget::handleDispatchCreated);
    connect(m_apiClient, &ApiClient::dispatchError, this, &DispatchWidget::handleDispatchError);
}

void DispatchWidget::setupTable()
{
    // 设置表头
    QStringList headers;
    headers << "ID" << "设备ID" << "批次ID" << "状态" << "开始时间" << "结束时间" << "操作员" << "优先级";
    m_model->setHorizontalHeaderLabels(headers);

    // 设置列宽
    ui->tableView->setColumnWidth(0, 60);     // ID列
    ui->tableView->setColumnWidth(1, 100);    // 设备ID列
    ui->tableView->setColumnWidth(2, 100);    // 批次ID列
    ui->tableView->setColumnWidth(3, 80);     // 状态列
    ui->tableView->setColumnWidth(4, 140);    // 开始时间列
    ui->tableView->setColumnWidth(5, 140);    // 结束时间列
    ui->tableView->setColumnWidth(6, 80);     // 操作员列
    ui->tableView->setColumnWidth(7, 60);     // 优先级列
}

void DispatchWidget::loadDispatchList()
{
    // 调用API获取调度列表
    m_apiClient->getDispatchList(m_currentPage, m_pageSize);

    // 更新分页信息
    updatePageInfo();
}

void DispatchWidget::onRefreshClicked()
{
    loadDispatchList();
}

void DispatchWidget::onSearchClicked()
{
    QString searchText = ui->searchEdit->text().trimmed();

    if (searchText.isEmpty()) {
        // 如果搜索框为空，加载全部调度列表
        loadDispatchList();
        return;
    }

    // 根据当前搜索模式执行相应的搜索
    if (m_currentSearchMode == SearchMode::SearchByEquipment) {
        m_apiClient->searchDispatchByEquipmentId(searchText);
    }
    else {
        m_apiClient->searchDispatchByLotId(searchText);
    }
}

void DispatchWidget::onSearchModeChanged(int index)
{
    m_currentSearchMode = static_cast<SearchMode>(ui->searchModeCombo->itemData(index).toInt());

    // 更新搜索框提示文本
    if (m_currentSearchMode == SearchMode::SearchByEquipment) {
        ui->searchEdit->setPlaceholderText("输入设备ID搜索调度");
    }
    else {
        ui->searchEdit->setPlaceholderText("输入批次ID搜索调度");
    }

    // 清空搜索框
    ui->searchEdit->clear();
}

void DispatchWidget::onCreateTaskClicked()
{
    // 创建并显示新建任务对话框
    CreateTaskDialog *dialog = new CreateTaskDialog(m_apiClient, this);
    connect(dialog, &CreateTaskDialog::taskCreated, this, &DispatchWidget::onTaskCreated);
    dialog->exec();
    dialog->deleteLater();
}

void DispatchWidget::onTableClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    // 获取选中行的调度状态
    QString status = m_model->data(m_model->index(index.row(), 3)).toString();

    // 更新状态样式
    updateStatusDisplay(index, status);
}

void DispatchWidget::onPrevPageClicked()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        loadDispatchList();
    }
}

void DispatchWidget::onNextPageClicked()
{
    int maxPage = (m_totalItems + m_pageSize - 1) / m_pageSize;

    if (m_currentPage < maxPage) {
        m_currentPage++;
        loadDispatchList();
    }
}

void DispatchWidget::onPageSizeChanged(int index)
{
    m_pageSize    = ui->pageSizeCombo->itemData(index).toInt();
    m_currentPage = 1;    // 重置到第一页
    loadDispatchList();
}

void DispatchWidget::updatePageInfo()
{
    int maxPage = (m_totalItems + m_pageSize - 1) / m_pageSize;
    maxPage     = maxPage < 1 ? 1 : maxPage;

    ui->pageInfoLabel->setText(QString("第 %1 / %2 页").arg(m_currentPage).arg(maxPage));

    // 更新按钮状态
    ui->prevButton->setEnabled(m_currentPage > 1);
    ui->nextButton->setEnabled(m_currentPage < maxPage);
}

void DispatchWidget::handleDispatchListReceived(const QList<Dispatch> &dispatchList, int totalCount)
{
    m_model->removeRows(0, m_model->rowCount());

    for (const Dispatch &dispatch: dispatchList) {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(dispatch.id());
        rowItems << new QStandardItem(dispatch.eqpId());
        rowItems << new QStandardItem(dispatch.lotId());
        rowItems << new QStandardItem(dispatch.status());
        rowItems << new QStandardItem(dispatch.startTime());
        rowItems << new QStandardItem(dispatch.endTime());
        rowItems << new QStandardItem(dispatch.operatorId());
        rowItems << new QStandardItem(dispatch.priority());

        m_model->appendRow(rowItems);

        // 对状态列应用样式
        int row = m_model->rowCount() - 1;
        updateStatusDisplay(m_model->index(row, 3), dispatch.status());
    }

    // 更新分页信息
    m_totalItems = totalCount;
    updatePageInfo();
}

void DispatchWidget::handleDispatchByEquipmentReceived(const QList<Dispatch> &dispatchList)
{
    m_model->removeRows(0, m_model->rowCount());

    for (const Dispatch &dispatch: dispatchList) {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(dispatch.id());
        rowItems << new QStandardItem(dispatch.eqpId());
        rowItems << new QStandardItem(dispatch.lotId());
        rowItems << new QStandardItem(dispatch.status());
        rowItems << new QStandardItem(dispatch.startTime());
        rowItems << new QStandardItem(dispatch.endTime());
        rowItems << new QStandardItem(dispatch.operatorId());
        rowItems << new QStandardItem(dispatch.priority());

        m_model->appendRow(rowItems);

        // 对状态列应用样式
        int row = m_model->rowCount() - 1;
        updateStatusDisplay(m_model->index(row, 3), dispatch.status());
    }

    // 更新分页信息
    m_totalItems  = dispatchList.size();
    m_currentPage = 1;
    updatePageInfo();
}

void DispatchWidget::handleDispatchByLotReceived(const QList<Dispatch> &dispatchList)
{
    m_model->removeRows(0, m_model->rowCount());

    for (const Dispatch &dispatch: dispatchList) {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(dispatch.id());
        rowItems << new QStandardItem(dispatch.eqpId());
        rowItems << new QStandardItem(dispatch.lotId());
        rowItems << new QStandardItem(dispatch.status());
        rowItems << new QStandardItem(dispatch.startTime());
        rowItems << new QStandardItem(dispatch.endTime());
        rowItems << new QStandardItem(dispatch.operatorId());
        rowItems << new QStandardItem(dispatch.priority());

        m_model->appendRow(rowItems);

        // 对状态列应用样式
        int row = m_model->rowCount() - 1;
        updateStatusDisplay(m_model->index(row, 3), dispatch.status());
    }

    // 更新分页信息
    m_totalItems  = dispatchList.size();
    m_currentPage = 1;
    updatePageInfo();
}

void DispatchWidget::handleDispatchCreated(bool success, const QString &message)
{
    if (success) {
        QMessageBox::information(this, "成功", message);
        // 刷新列表以显示新创建的调度任务
        loadDispatchList();
    }
    else {
        QMessageBox::warning(this, "错误", message);
    }
}

void DispatchWidget::handleDispatchError(const QString &errorMessage)
{
    QMessageBox::warning(this, "调度数据错误", errorMessage);
}

void DispatchWidget::onTaskCreated(const Dispatch &dispatch)
{
    // 调用API创建新的调度任务
    m_apiClient->createDispatch(dispatch);
}

void DispatchWidget::updateStatusDisplay(const QModelIndex &index, const QString &status)
{
    if (!index.isValid())
        return;

    // 根据状态设置不同的颜色
    QStandardItem *item = m_model->itemFromIndex(index);
    if (!item)
        return;

    QBrush brush;

    if (status == "未开始") {
        brush = QBrush(QColor("#409EFF"));    // 蓝色
    }
    else if (status == "进行中") {
        brush = QBrush(QColor("#E6A23C"));    // 橙色
    }
    else if (status == "已完成") {
        brush = QBrush(QColor("#67C23A"));    // 绿色
    }
    else if (status == "已取消") {
        brush = QBrush(QColor("#909399"));    // 灰色
    }
    else if (status == "异常") {
        brush = QBrush(QColor("#F56C6C"));    // 红色
    }
    else {
        brush = QBrush(QColor("#606266"));    // 默认深灰色
    }

    item->setForeground(brush);
    item->setFont(QFont("", -1, QFont::Bold));
}
