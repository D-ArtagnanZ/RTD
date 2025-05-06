#include "createtaskdialog.h"
#include "ui_createtaskdialog.h"

CreateTaskDialog::CreateTaskDialog(ApiClient *apiClient, QWidget *parent)
    : QDialog(parent), ui(new Ui::CreateTaskDialog), m_apiClient(apiClient)
{
    ui->setupUi(this);

    setupUi();
    setupConnections();

    // 加载设备和批次数据
    loadEquipmentData();
    loadLotData();
}

CreateTaskDialog::~CreateTaskDialog()
{
    delete ui;
}

void CreateTaskDialog::setupUi()
{
    // 设置窗口属性
    setWindowTitle("创建调度任务");
    setModal(true);

    // 设置优先级选项
    ui->priorityComboBox->addItem("低", "low");
    ui->priorityComboBox->addItem("中", "medium");
    ui->priorityComboBox->addItem("高", "high");
    ui->priorityComboBox->addItem("紧急", "urgent");

    // 设置状态选项
    ui->statusComboBox->addItem("未开始", "未开始");
    ui->statusComboBox->addItem("进行中", "进行中");

    // 设置开始时间和结束时间默认值
    QDateTime currentDateTime = QDateTime::currentDateTime();
    ui->startTimeEdit->setDateTime(currentDateTime);
    ui->endTimeEdit->setDateTime(currentDateTime.addDays(1));

    // 设置按钮样式
    ui->createButton->setProperty("class", "actionButton");
    ui->cancelButton->setProperty("class", "secondaryButton");
}

void CreateTaskDialog::setupConnections()
{
    // 按钮连接
    connect(ui->createButton, &QPushButton::clicked, this, &CreateTaskDialog::onCreateButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &CreateTaskDialog::onCancelButtonClicked);

    // API客户端连接
    connect(m_apiClient, &ApiClient::equipmentListReceived, this, &CreateTaskDialog::handleEquipmentListReceived);
    connect(m_apiClient, &ApiClient::lotListReceived, this, &CreateTaskDialog::handleLotListReceived);
    connect(m_apiClient, &ApiClient::equipmentError, this, &CreateTaskDialog::handleEquipmentError);
    connect(m_apiClient, &ApiClient::lotError, this, &CreateTaskDialog::handleLotError);
}

void CreateTaskDialog::onCreateButtonClicked()
{
    // 验证表单数据
    if (ui->equipmentComboBox->currentIndex() < 0) {
        QMessageBox::warning(this, "表单错误", "请选择设备");
        return;
    }

    if (ui->lotComboBox->currentIndex() < 0) {
        QMessageBox::warning(this, "表单错误", "请选择批次");
        return;
    }

    if (ui->operatorIdEdit->text().isEmpty()) {
        QMessageBox::warning(this, "表单错误", "请输入操作员ID");
        return;
    }

    if (ui->startTimeEdit->dateTime() >= ui->endTimeEdit->dateTime()) {
        QMessageBox::warning(this, "表单错误", "开始时间必须早于结束时间");
        return;
    }

    // 创建调度任务对象
    Dispatch dispatch;

    // 设置选中的设备ID和批次ID
    int equipmentIndex = ui->equipmentComboBox->currentIndex();
    int lotIndex       = ui->lotComboBox->currentIndex();

    if (equipmentIndex >= 0 && equipmentIndex < m_equipmentList.size()) {
        dispatch.setField("EQP_ID", m_equipmentList[equipmentIndex].id());
    }

    if (lotIndex >= 0 && lotIndex < m_lotList.size()) {
        dispatch.setField("LOT_ID", m_lotList[lotIndex].id());
    }

    // 设置其他字段
    dispatch.setField("STATUS", ui->statusComboBox->currentData().toString());
    dispatch.setField("START_TIME", ui->startTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
    dispatch.setField("END_TIME", ui->endTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
    dispatch.setField("OPERATOR_ID", ui->operatorIdEdit->text());
    dispatch.setField("PRIORITY", ui->priorityComboBox->currentData().toString());
    dispatch.setField("REMARKS", ui->remarksEdit->toPlainText());

    // 发出任务创建信号
    emit taskCreated(dispatch);

    // 关闭对话框
    accept();
}

void CreateTaskDialog::onCancelButtonClicked()
{
    // 关闭对话框
    reject();
}

void CreateTaskDialog::loadEquipmentData()
{
    // 请求设备列表，使用较大的页面大小加载更多数据
    m_apiClient->getEquipmentList(1, 100);
}

void CreateTaskDialog::loadLotData()
{
    // 请求批次列表，使用较大的页面大小加载更多数据
    m_apiClient->getLotList(1, 100);
}

void CreateTaskDialog::handleEquipmentListReceived(const QList<Equipment> &equipmentList, int totalCount)
{
    // 保存设备列表
    m_equipmentList = equipmentList;

    // 更新设备下拉框
    ui->equipmentComboBox->clear();

    for (const Equipment &equipment: equipmentList) {
        QString displayText = QString("%1 (%2)").arg(equipment.name()).arg(equipment.id());
        ui->equipmentComboBox->addItem(displayText, equipment.id());
    }
}

void CreateTaskDialog::handleLotListReceived(const QList<Lot> &lotList, int totalCount)
{
    // 保存批次列表
    m_lotList = lotList;

    // 更新批次下拉框
    ui->lotComboBox->clear();

    for (const Lot &lot: lotList) {
        QString displayText = QString("%1 (%2)").arg(lot.name()).arg(lot.id());
        ui->lotComboBox->addItem(displayText, lot.id());
    }
}

void CreateTaskDialog::handleEquipmentError(const QString &errorMessage)
{
    QMessageBox::warning(this, "设备数据错误", errorMessage);
}

void CreateTaskDialog::handleLotError(const QString &errorMessage)
{
    QMessageBox::warning(this, "批次数据错误", errorMessage);
}
