#include "MainWindow.h"
#include "ApiClient.h"
#include "models/DispatchModel.h"
#include "models/EquipmentModel.h"
#include "models/LotModel.h"
#include "utils/StyleHelper.h"
#include "views/DispatchView.h"
#include "views/EquipmentListView.h"
#include "views/LotListView.h"

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_darkTheme(false)
{
    setWindowTitle("芯片测试派工系统");
    setMinimumSize(1024, 768);

    // 创建API客户端
    m_apiClient = new ApiClient("", this);    // 初始化时不设置URL，等待配置加载

    // 加载API配置
    if (!m_apiClient->loadConfig("config.json")) {
        // 如果配置加载失败，使用默认URL
        m_apiClient->setBaseUrl("http://localhost:8080/api");
        qWarning() << "配置加载失败，使用默认服务器地址";
    }

    // 设置模型和视图
    setupModels();
    setupUi();
    createActions();
    createMenus();

    // 设置刷新定时器 (5分钟)
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshAllData);
    m_refreshTimer->start(5 * 60 * 1000);

    // 初始加载数据
    QTimer::singleShot(100, this, &MainWindow::refreshAllData);

    // 显示就绪状态
    statusBar()->showMessage("系统就绪");

    // 应用样式
    StyleHelper::applyStylesheet(":/styles/main.qss");
}

MainWindow::~MainWindow()
{
    // 资源会自动释放
}

void MainWindow::setupUi()
{
    // 创建标签页容器
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // 创建派工信息视图
    m_dispatchView = new DispatchView(m_dispatchModel, this);
    m_tabWidget->addTab(m_dispatchView, "派工信息");

    // 创建产品批次视图
    m_lotListView = new LotListView(m_lotModel, this);
    m_tabWidget->addTab(m_lotListView, "产品批次");

    // 创建机台设备视图
    m_equipmentView = new EquipmentListView(m_equipmentModel, this);
    m_tabWidget->addTab(m_equipmentView, "机台列表");

    // 设置状态栏时间标签
    m_timeLabel = new QLabel(this);
    m_timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    statusBar()->addPermanentWidget(m_timeLabel);

    // 添加定时器更新时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        m_timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    });
    timer->start(1000);
}

void MainWindow::setupModels()
{
    // 创建数据模型
    m_lotModel       = new LotModel(m_apiClient, this);
    m_equipmentModel = new EquipmentModel(m_apiClient, this);
    m_dispatchModel  = new DispatchModel(m_apiClient, this);
}

void MainWindow::createActions()
{
    // 文件菜单动作
    m_refreshAction = new QAction("刷新数据", this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshAllData);

    m_exitAction = new QAction("退出", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, qApp, &QApplication::quit);

    // 视图菜单动作
    m_themeAction = new QAction("切换暗色主题", this);
    connect(m_themeAction, &QAction::triggered, this, &MainWindow::switchTheme);

    // 工具菜单动作
    m_settingsAction = new QAction("设置", this);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::showSettings);

    // 帮助菜单动作
    m_aboutAction = new QAction("关于", this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::createMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件");
    fileMenu->addAction(m_refreshAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    // 视图菜单
    QMenu *viewMenu = menuBar()->addMenu("视图");
    viewMenu->addAction(m_themeAction);

    // 工具菜单
    QMenu *toolsMenu = menuBar()->addMenu("工具");
    toolsMenu->addAction(m_settingsAction);

    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::refreshAllData()
{
    // 刷新所有数据模型
    m_lotModel->refresh();
    m_equipmentModel->refresh();
    m_dispatchModel->refresh();

    // 更新状态栏
    statusBar()->showMessage("数据已刷新", 3000);
}

void MainWindow::switchTheme()
{
    m_darkTheme = !m_darkTheme;

    if (m_darkTheme) {
        StyleHelper::applyDarkTheme();
        m_themeAction->setText("切换亮色主题");
    }
    else {
        StyleHelper::applyStylesheet(":/styles/main.qss");
        m_themeAction->setText("切换暗色主题");
    }
}

void MainWindow::showSettings()
{
    // 显示设置对话框（示例实现）
    QMessageBox::information(this, "设置", "设置功能正在开发中");
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "关于", "芯片测试派工系统 1.0.0\n\n"
                                     "用于芯片测试厂区的实时派工管理\n\n"
                                     "© 2024 版权所有");
}

void MainWindow::handleApiConnectionStatus(bool connected)
{
    if (connected) {
        statusBar()->showMessage("已连接到服务器", 3000);
    }
    else {
        statusBar()->showMessage("服务器连接失败", 3000);
    }
}
