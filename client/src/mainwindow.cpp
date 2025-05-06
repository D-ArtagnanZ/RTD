#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建API客户端
    m_apiClient = new ApiClient(this);

    // 设置UI
    setupUi();
    setupConnections();
    loadStyleSheet();

    // 默认显示设备页
    onNavButtonClicked(0);

    setWindowTitle("RTD+ 芯片测试厂派工系统");
    resize(1280, 800);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUi()
{
    // 创建内容页
    m_equipmentWidget = new EquipmentWidget(m_apiClient, this);
    m_lotWidget       = new LotWidget(m_apiClient, this);
    m_dispatchWidget  = new DispatchWidget(m_apiClient, this);

    ui->contentStack->addWidget(m_equipmentWidget);
    ui->contentStack->addWidget(m_lotWidget);
    ui->contentStack->addWidget(m_dispatchWidget);

    // 设置导航按钮组
    m_navButtons = new QButtonGroup(this);
    m_navButtons->addButton(ui->equipmentButton, 0);
    m_navButtons->addButton(ui->lotButton, 1);
    m_navButtons->addButton(ui->dispatchButton, 2);
    m_navButtons->addButton(ui->settingsButton, 3);

    // 设置按钮样式
    ui->equipmentButton->setCheckable(true);
    ui->lotButton->setCheckable(true);
    ui->dispatchButton->setCheckable(true);
    ui->settingsButton->setCheckable(true);

    ui->equipmentButton->setProperty("class", "navButton");
    ui->lotButton->setProperty("class", "navButton");
    ui->dispatchButton->setProperty("class", "navButton");
    ui->settingsButton->setProperty("class", "navButton");

    // 使用Qt内置图标代替资源文件中的图标
    ui->equipmentButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    ui->lotButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
    ui->dispatchButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    ui->settingsButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));

    // 设置图标大小
    ui->equipmentButton->setIconSize(QSize(24, 24));
    ui->lotButton->setIconSize(QSize(24, 24));
    ui->dispatchButton->setIconSize(QSize(24, 24));
    ui->settingsButton->setIconSize(QSize(24, 24));

    // 设置logo
    ui->logoLabel->setPixmap(QPixmap(":/images/logo.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::setupConnections()
{
    // 修复Qt6中的QButtonGroup信号连接
    // 在Qt6中，使用新的连接语法
    connect(m_navButtons, &QButtonGroup::idClicked, this, &MainWindow::onNavButtonClicked);

    // 网络错误处理
    connect(m_apiClient, &ApiClient::networkError, this, &MainWindow::handleNetworkError);
}

void MainWindow::loadStyleSheet()
{
    QFile file(":/styles/main.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        setStyleSheet(styleSheet);
        file.close();
    }
}

void MainWindow::onNavButtonClicked(int id)
{
    // 切换到相应的内容页
    switch (id) {
        case 0:    // 设备页
            ui->contentStack->setCurrentWidget(m_equipmentWidget);
            break;

        case 1:    // 批次页
            ui->contentStack->setCurrentWidget(m_lotWidget);
            break;

        case 2:    // 调度页
            ui->contentStack->setCurrentWidget(m_dispatchWidget);
            break;

        case 3:    // 设置页
            onServerSettingsClicked();
            ui->equipmentButton->setChecked(true);    // 返回到设备页
            break;
    }
}

void MainWindow::onServerSettingsClicked()
{
    QString currentUrl = m_apiClient->getServerUrl();
    bool    ok;
    QString newUrl = QInputDialog::getText(this, "服务器设置", "请输入服务器URL:", QLineEdit::Normal, currentUrl, &ok);

    if (ok && !newUrl.isEmpty()) {
        m_apiClient->setServerUrl(newUrl);
        QMessageBox::information(this, "设置已更新", "服务器URL已更新为: " + newUrl);
    }
}

void MainWindow::handleNetworkError(const QString &errorMessage)
{
    QMessageBox::warning(this, "网络错误", errorMessage);
}
