#include "equipmentwidget.h"
#include "ui_equipmentwidget.h"

EquipmentWidget::EquipmentWidget(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), ui(new Ui::EquipmentWidget), m_apiClient(apiClient), m_currentPage(1), m_pageSize(10), m_totalItems(0)
{
    ui->setupUi(this);

    // 设置表格模型
    m_model = new QStandardItemModel(this);

    setupUi();
    setupConnections();
    setupTable();

    // 初始加载设备列表
    loadEquipmentList();
}

EquipmentWidget::~EquipmentWidget()
{
    delete ui;
}

void EquipmentWidget::setupUi()
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
    ui->searchEdit->setPlaceholderText("输入设备ID或名称搜索");
}

void EquipmentWidget::setupConnections()
{
    // 按钮连接
    connect(ui->refreshButton, &QPushButton::clicked, this, &EquipmentWidget::onRefreshClicked);
    connect(ui->searchButton, &QPushButton::clicked, this, &EquipmentWidget::onSearchClicked);
    connect(ui->prevButton, &QPushButton::clicked, this, &EquipmentWidget::onPrevPageClicked);
    connect(ui->nextButton, &QPushButton::clicked, this, &EquipmentWidget::onNextPageClicked);

    // 表格选择连接
    connect(ui->tableView, &QTableView::clicked, this, &EquipmentWidget::onTableClicked);

    // 页面大小连接
    connect(ui->pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EquipmentWidget::onPageSizeChanged);

    // API客户端连接
    connect(m_apiClient, &ApiClient::equipmentReceived, this, &EquipmentWidget::handleEquipmentReceived);
    connect(m_apiClient, &ApiClient::equipmentListReceived, this, &EquipmentWidget::handleEquipmentListReceived);
    connect(m_apiClient, &ApiClient::equipmentError, this, &EquipmentWidget::handleEquipmentError);
}

void EquipmentWidget::setupTable()
{
    // 设置表头
    QStringList headers;
    headers << "ID" << "名称" << "类型" << "状态" << "位置" << "描述";
    m_model->setHorizontalHeaderLabels(headers);

    // 设置列宽
    ui->tableView->setColumnWidth(0, 100);    // ID列
    ui->tableView->setColumnWidth(1, 150);    // 名称列
    ui->tableView->setColumnWidth(2, 100);    // 类型列
    ui->tableView->setColumnWidth(3, 100);    // 状态列
    ui->tableView->setColumnWidth(4, 120);    // 位置列
}

void EquipmentWidget::loadEquipmentList()
{
    // 调用API获取设备列表
    m_apiClient->getEquipmentList(m_currentPage, m_pageSize);

    // 更新分页信息
    updatePageInfo();
}

void EquipmentWidget::onRefreshClicked()
{
    loadEquipmentList();
}

void EquipmentWidget::onSearchClicked()
{
    QString searchText = ui->searchEdit->text().trimmed();

    if (searchText.isEmpty()) {
        // 如果搜索框为空，加载全部设备列表
        loadEquipmentList();
        return;
    }

    // 调用API搜索设备
    m_apiClient->getEquipmentById(searchText);
}

void EquipmentWidget::onTableClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    // 获取选中行的设备ID
    QString equipmentId = m_model->data(m_model->index(index.row(), 0)).toString();

    // 调用API获取设备详情
    m_apiClient->getEquipmentById(equipmentId);
}

void EquipmentWidget::onPrevPageClicked()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        loadEquipmentList();
    }
}

void EquipmentWidget::onNextPageClicked()
{
    int maxPage = (m_totalItems + m_pageSize - 1) / m_pageSize;

    if (m_currentPage < maxPage) {
        m_currentPage++;
        loadEquipmentList();
    }
}

void EquipmentWidget::onPageSizeChanged(int index)
{
    m_pageSize    = ui->pageSizeCombo->itemData(index).toInt();
    m_currentPage = 1;    // 重置到第一页
    loadEquipmentList();
}

void EquipmentWidget::updatePageInfo()
{
    int maxPage = (m_totalItems + m_pageSize - 1) / m_pageSize;
    maxPage     = maxPage < 1 ? 1 : maxPage;

    ui->pageInfoLabel->setText(QString("第 %1 / %2 页").arg(m_currentPage).arg(maxPage));

    // 更新按钮状态
    ui->prevButton->setEnabled(m_currentPage > 1);
    ui->nextButton->setEnabled(m_currentPage < maxPage);
}

void EquipmentWidget::handleEquipmentReceived(const Equipment &equipment)
{
    m_selectedEquipment = equipment;
    updateEquipmentDetails(equipment);

    // 如果是搜索结果，更新表格只显示这一个设备
    m_model->removeRows(0, m_model->rowCount());

    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(equipment.id());
    rowItems << new QStandardItem(equipment.name());
    rowItems << new QStandardItem(equipment.type());
    rowItems << new QStandardItem(equipment.status());
    rowItems << new QStandardItem(equipment.location());
    rowItems << new QStandardItem(equipment.description());

    m_model->appendRow(rowItems);

    // 更新分页信息
    m_totalItems  = 1;
    m_currentPage = 1;
    updatePageInfo();
}

void EquipmentWidget::handleEquipmentListReceived(const QList<Equipment> &equipmentList, int totalCount)
{
    m_model->removeRows(0, m_model->rowCount());

    for (const Equipment &equipment: equipmentList) {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(equipment.id());
        rowItems << new QStandardItem(equipment.name());
        rowItems << new QStandardItem(equipment.type());
        rowItems << new QStandardItem(equipment.status());
        rowItems << new QStandardItem(equipment.location());
        rowItems << new QStandardItem(equipment.description());

        m_model->appendRow(rowItems);
    }

    // 清空详情显示
    ui->idValueLabel->setText("");
    ui->nameValueLabel->setText("");
    ui->typeValueLabel->setText("");
    ui->statusValueLabel->setText("");
    ui->locationValueLabel->setText("");
    ui->descriptionValueLabel->setText("");

    // 更新分页信息
    m_totalItems = totalCount;
    updatePageInfo();
}

void EquipmentWidget::handleEquipmentError(const QString &errorMessage)
{
    QMessageBox::warning(this, "设备数据错误", errorMessage);
}

void EquipmentWidget::updateEquipmentDetails(const Equipment &equipment)
{
    // 更新详情显示
    ui->idValueLabel->setText(equipment.id());
    ui->nameValueLabel->setText(equipment.name());
    ui->typeValueLabel->setText(equipment.type());
    ui->locationValueLabel->setText(equipment.location());
    ui->descriptionValueLabel->setText(equipment.description());

    updateStatusDisplay(equipment.status());
}

void EquipmentWidget::updateStatusDisplay(const QString &status)
{
    // 根据状态设置不同的显示样式
    ui->statusValueLabel->setText(status);

    QString styleSheet;

    if (status == "运行中") {
        styleSheet = "color: #67C23A; font-weight: bold;";    // 绿色
    }
    else if (status == "空闲") {
        styleSheet = "color: #409EFF; font-weight: bold;";    // 蓝色
    }
    else if (status == "故障") {
        styleSheet = "color: #F56C6C; font-weight: bold;";    // 红色
    }
    else if (status == "维护中") {
        styleSheet = "color: #E6A23C; font-weight: bold;";    // 橙色
    }
    else {
        styleSheet = "color: #606266; font-weight: bold;";    // 默认灰色
    }

    ui->statusValueLabel->setStyleSheet(styleSheet);
}
