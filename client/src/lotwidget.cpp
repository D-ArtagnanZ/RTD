#include "lotwidget.h"
#include "ui_lotwidget.h"

LotWidget::LotWidget(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), ui(new Ui::LotWidget), m_apiClient(apiClient), m_currentPage(1), m_pageSize(10), m_totalItems(0)
{
    ui->setupUi(this);

    // 设置表格模型
    m_model = new QStandardItemModel(this);

    setupUi();
    setupConnections();
    setupTable();

    // 初始加载批次列表
    loadLotList();
}

LotWidget::~LotWidget()
{
    delete ui;
}

void LotWidget::setupUi()
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

    // 设置按钮样式
    ui->refreshButton->setProperty("class", "actionButton");
    ui->searchButton->setProperty("class", "actionButton");
    ui->prevButton->setProperty("class", "pageButton");
    ui->nextButton->setProperty("class", "pageButton");

    // 设置搜索框样式
    ui->searchEdit->setProperty("class", "searchInput");
    ui->searchEdit->setPlaceholderText("输入批次ID或名称搜索");
}

void LotWidget::setupConnections()
{
    // 按钮连接
    connect(ui->refreshButton, &QPushButton::clicked, this, &LotWidget::onRefreshClicked);
    connect(ui->searchButton, &QPushButton::clicked, this, &LotWidget::onSearchClicked);
    connect(ui->prevButton, &QPushButton::clicked, this, &LotWidget::onPrevPageClicked);
    connect(ui->nextButton, &QPushButton::clicked, this, &LotWidget::onNextPageClicked);

    // 表格选择连接
    connect(ui->tableView, &QTableView::clicked, this, &LotWidget::onTableClicked);

    // 页面大小连接
    connect(ui->pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LotWidget::onPageSizeChanged);

    // API客户端连接
    connect(m_apiClient, &ApiClient::lotReceived, this, &LotWidget::handleLotReceived);
    connect(m_apiClient, &ApiClient::lotListReceived, this, &LotWidget::handleLotListReceived);
    connect(m_apiClient, &ApiClient::lotError, this, &LotWidget::handleLotError);
}

void LotWidget::setupTable()
{
    // 设置表头
    QStringList headers;
    headers << "ID" << "名称" << "产品" << "状态" << "数量" << "优先级" << "开始日期" << "截止日期";
    m_model->setHorizontalHeaderLabels(headers);

    // 设置列宽
    ui->tableView->setColumnWidth(0, 80);     // ID列
    ui->tableView->setColumnWidth(1, 150);    // 名称列
    ui->tableView->setColumnWidth(2, 100);    // 产品列
    ui->tableView->setColumnWidth(3, 80);     // 状态列
    ui->tableView->setColumnWidth(4, 60);     // 数量列
    ui->tableView->setColumnWidth(5, 80);     // 优先级列
    ui->tableView->setColumnWidth(6, 120);    // 开始日期列
    ui->tableView->setColumnWidth(7, 120);    // 截止日期列
}

void LotWidget::loadLotList()
{
    // 调用API获取批次列表
    m_apiClient->getLotList(m_currentPage, m_pageSize);

    // 更新分页信息
    updatePageInfo();
}

void LotWidget::onRefreshClicked()
{
    loadLotList();
}

void LotWidget::onSearchClicked()
{
    QString searchText = ui->searchEdit->text().trimmed();

    if (searchText.isEmpty()) {
        // 如果搜索框为空，加载全部批次列表
        loadLotList();
        return;
    }

    // 调用API搜索批次
    m_apiClient->getLotById(searchText);
}

void LotWidget::onTableClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    // 获取选中行的批次ID
    QString lotId = m_model->data(m_model->index(index.row(), 0)).toString();

    // 调用API获取批次详情
    m_apiClient->getLotById(lotId);
}

void LotWidget::onPrevPageClicked()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        loadLotList();
    }
}

void LotWidget::onNextPageClicked()
{
    int maxPage = (m_totalItems + m_pageSize - 1) / m_pageSize;

    if (m_currentPage < maxPage) {
        m_currentPage++;
        loadLotList();
    }
}

void LotWidget::onPageSizeChanged(int index)
{
    m_pageSize    = ui->pageSizeCombo->itemData(index).toInt();
    m_currentPage = 1;    // 重置到第一页
    loadLotList();
}

void LotWidget::updatePageInfo()
{
    int maxPage = (m_totalItems + m_pageSize - 1) / m_pageSize;
    maxPage     = maxPage < 1 ? 1 : maxPage;

    ui->pageInfoLabel->setText(QString("第 %1 / %2 页").arg(m_currentPage).arg(maxPage));

    // 更新按钮状态
    ui->prevButton->setEnabled(m_currentPage > 1);
    ui->nextButton->setEnabled(m_currentPage < maxPage);
}

void LotWidget::handleLotReceived(const Lot &lot)
{
    updateLotDetails(lot);

    // 如果是搜索结果，更新表格只显示这一个批次
    m_model->removeRows(0, m_model->rowCount());

    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(lot.id());
    rowItems << new QStandardItem(lot.name());
    rowItems << new QStandardItem(lot.product());
    rowItems << new QStandardItem(lot.status());
    rowItems << new QStandardItem(QString::number(lot.quantity()));
    rowItems << new QStandardItem(lot.priority());
    rowItems << new QStandardItem(lot.startDate());
    rowItems << new QStandardItem(lot.dueDate());

    m_model->appendRow(rowItems);

    // 更新分页信息
    m_totalItems  = 1;
    m_currentPage = 1;
    updatePageInfo();
}

void LotWidget::handleLotListReceived(const QList<Lot> &lotList, int totalCount)
{
    m_model->removeRows(0, m_model->rowCount());

    for (const Lot &lot: lotList) {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(lot.id());
        rowItems << new QStandardItem(lot.name());
        rowItems << new QStandardItem(lot.product());
        rowItems << new QStandardItem(lot.status());
        rowItems << new QStandardItem(QString::number(lot.quantity()));
        rowItems << new QStandardItem(lot.priority());
        rowItems << new QStandardItem(lot.startDate());
        rowItems << new QStandardItem(lot.dueDate());

        m_model->appendRow(rowItems);
    }

    // 清空详情显示
    ui->idValueLabel->setText("");
    ui->nameValueLabel->setText("");
    ui->productValueLabel->setText("");
    ui->statusValueLabel->setText("");
    ui->quantityValueLabel->setText("");
    ui->priorityValueLabel->setText("");
    ui->startDateValueLabel->setText("");
    ui->dueDateValueLabel->setText("");

    // 更新分页信息
    m_totalItems = totalCount;
    updatePageInfo();
}

void LotWidget::handleLotError(const QString &errorMessage)
{
    QMessageBox::warning(this, "批次数据错误", errorMessage);
}

void LotWidget::updateLotDetails(const Lot &lot)
{
    // 更新详情显示
    ui->idValueLabel->setText(lot.id());
    ui->nameValueLabel->setText(lot.name());
    ui->productValueLabel->setText(lot.product());
    ui->quantityValueLabel->setText(QString::number(lot.quantity()));
    ui->priorityValueLabel->setText(lot.priority());
    ui->startDateValueLabel->setText(lot.startDate());
    ui->dueDateValueLabel->setText(lot.dueDate());

    updateStatusDisplay(lot.status());
}

void LotWidget::updateStatusDisplay(const QString &status)
{
    // 根据状态设置不同的显示样式
    ui->statusValueLabel->setText(status);

    QString styleSheet;

    if (status == "待处理") {
        styleSheet = "color: #409EFF; font-weight: bold;";    // 蓝色
    }
    else if (status == "进行中") {
        styleSheet = "color: #E6A23C; font-weight: bold;";    // 橙色
    }
    else if (status == "已完成") {
        styleSheet = "color: #67C23A; font-weight: bold;";    // 绿色
    }
    else if (status == "异常") {
        styleSheet = "color: #F56C6C; font-weight: bold;";    // 红色
    }
    else {
        styleSheet = "color: #606266; font-weight: bold;";    // 默认灰色
    }

    ui->statusValueLabel->setStyleSheet(styleSheet);
}
