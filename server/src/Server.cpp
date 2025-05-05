#include "Server.h"
#include "api/ApiManager.h"
#include "config/ConfigManager.h"
#include "db/DatabaseManager.h"

#include <QDebug>
#include <QHostAddress>
#include <stdexcept>

Server::Server(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_running(false)
{
    // 连接信号槽
    connect(&m_httpServer, &HttpServer::requestReceived, this, &Server::handleHttpRequest);

    // 设置健康检查定时器
    connect(&m_healthCheckTimer, &QTimer::timeout, this, &Server::checkDatabasePools);
}

Server::~Server()
{
    if (m_running) {
        stop();
    }
}

bool Server::initialize(const QString &configPath)
{
    if (m_initialized) {
        qDebug() << "服务器已经初始化";
        return true;
    }

    qDebug() << "初始化服务器...";

    try {
        // 初始化配置
        if (!initializeConfig(configPath)) {
            return false;
        }

        // 初始化数据库
        if (!initializeDatabase()) {
            return false;
        }

        // 初始化API管理器
        if (!initializeApiManager()) {
            return false;
        }

        // 初始化HTTP服务器
        if (!initializeHttpServer()) {
            return false;
        }

        m_initialized = true;
        qDebug() << "服务器初始化完成";
        return true;
    }
    catch (const std::exception &e) {
        qCritical() << "初始化服务器失败:" << e.what();
        return false;
    }
}

bool Server::start()
{
    if (!m_initialized && !initialize()) {
        return false;
    }

    if (m_running) {
        qDebug() << "服务器已经运行";
        return true;
    }

    // 获取服务器配置
    QString hostAddress = m_serverConfig.value("server").toMap().value("host", "0.0.0.0").toString();
    int     port        = m_serverConfig.value("server").toMap().value("port", 8080).toInt();

    qDebug() << "启动服务器在" << hostAddress << ":" << port;

    // 启动HTTP服务器
    if (!m_httpServer.start(port, hostAddress)) {
        qCritical() << "启动HTTP服务器失败";
        return false;
    }

    // 启动健康检查定时器
    int healthCheckInterval = m_serverConfig.value("server").toMap().value("health_check_interval", 60000).toInt();
    m_healthCheckTimer.start(healthCheckInterval);

    m_running = true;
    qDebug() << "服务器成功启动";
    return true;
}

void Server::stop()
{
    if (!m_running) {
        return;
    }

    qDebug() << "停止服务器...";

    // 停止健康检查定时器
    m_healthCheckTimer.stop();

    // 停止HTTP服务器
    m_httpServer.stop();

    // 关闭所有数据库连接
    try {
        DatabaseManager::instance().closeAllConnections();
    }
    catch (const std::exception &e) {
        qWarning() << "关闭数据库连接时发生错误:" << e.what();
    }

    m_running = false;
    qDebug() << "服务器已停止";
}

bool Server::isRunning() const
{
    return m_running;
}

void Server::handleHttpRequest(const HttpRequest &request)
{
    try {
        // 将请求转发给API管理器
        HttpResponse response = ApiManager::instance().handleApiRequest(request);

        // 在这里可以添加全局响应处理，如CORS头处理等
        if (m_serverConfig.value("cors").toMap().value("enabled", false).toBool()) {
            response.setHeader("Access-Control-Allow-Origin", "*");
            response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        }

        // 返回响应
        // 注意：实际响应会由HttpServer处理
    }
    catch (const std::exception &e) {
        qCritical() << "处理HTTP请求失败:" << e.what();
    }
}

void Server::checkDatabasePools()
{
    try {
        // 获取数据库连接池状态
        QVariantMap poolsStats = DatabaseManager::instance().getPoolsStats();

        // 打印连接池状态
        QVariantList pools = poolsStats.value("pools").toList();
        for (const QVariant &pool: pools) {
            QVariantMap poolStats = pool.toMap();
            qDebug() << "连接池:" << poolStats.value("poolName").toString()
                     << "活跃连接:" << poolStats.value("activeConnections").toInt()
                     << "空闲连接:" << poolStats.value("idleConnections").toInt();
        }
    }
    catch (const std::exception &e) {
        qWarning() << "检查数据库连接池失败:" << e.what();
    }
}

bool Server::initializeConfig(const QString &configPath)
{
    qDebug() << "加载配置文件:" << configPath;

    // 加载服务器配置
    if (!ConfigManager::instance().loadConfigs(configPath)) {
        qCritical() << "加载配置失败";
        return false;
    }

    // 获取配置
    m_serverConfig = ConfigManager::instance().getServerConfig();

    return true;
}

bool Server::initializeDatabase()
{
    qDebug() << "初始化数据库连接...";

    try {
        // 获取数据库配置
        QVariantMap dbConfig = ConfigManager::instance().getDatabaseConfig();

        // 初始化数据库管理器
        DatabaseManager::instance().initialize(dbConfig);

        return true;
    }
    catch (const std::exception &e) {
        qCritical() << "初始化数据库失败:" << e.what();
        return false;
    }
}

bool Server::initializeApiManager()
{
    qDebug() << "初始化API管理器...";

    try {
        // 获取API配置
        QVariantMap apiConfig = ConfigManager::instance().getApiConfig();

        // 初始化API管理器
        ApiManager::instance().initialize(apiConfig);

        return true;
    }
    catch (const std::exception &e) {
        qCritical() << "初始化API管理器失败:" << e.what();
        return false;
    }
}

bool Server::initializeHttpServer()
{
    qDebug() << "初始化HTTP服务器...";

    // 注册自定义NotFound处理函数
    m_httpServer.setNotFoundHandler([](const HttpRequest &) {
        QVariantMap errorResponse;
        errorResponse["success"] = false;
        errorResponse["error"]   = "资源未找到";

        return HttpResponse::fromMap(errorResponse, 404);
    });

    return true;
}
