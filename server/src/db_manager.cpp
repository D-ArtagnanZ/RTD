#include "db_manager.h"
#include <fstream>
#include <iostream>
#include <soci/odbc/soci-odbc.h>

DbManager &DbManager::instance()
{
    static DbManager instance;
    return instance;
}

bool DbManager::initialize(const std::string &configPath)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_initialized) {
        return true;
    }

    try {
        // 读取数据源配置文件
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            std::cerr << "无法打开数据源配置文件: " << configPath << std::endl;
            return false;
        }

        json config;
        configFile >> config;

        // 解析每个数据源的配置
        for (auto &[name, source]: config.items()) {
            DataSourceConfig sourceConfig;
            sourceConfig.dsnPath     = source["dsn_path"];
            sourceConfig.minConn     = source["min_conn"];
            sourceConfig.maxConn     = source["max_conn"];
            sourceConfig.connTimeout = source["conn_timeout"];

            // 加载DSN文件内容
            sourceConfig.connectionString = loadDsn(sourceConfig.dsnPath);
            if (sourceConfig.connectionString.empty()) {
                std::cerr << "无法加载DSN文件: " << sourceConfig.dsnPath << std::endl;
                continue;
            }

            m_configs[name] = sourceConfig;

            // 初始化连接池
            std::vector<std::shared_ptr<soci::session>> pool;
            for (int i = 0; i < sourceConfig.minConn; i++) {
                try {
                    auto session = std::make_shared<soci::session>(soci::odbc, sourceConfig.connectionString);
                    pool.push_back(session);
                }
                catch (const std::exception &e) {
                    std::cerr << "创建数据库连接失败: " << e.what() << std::endl;
                }
            }

            m_connectionPools[name] = std::move(pool);
        }

        m_initialized = true;
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "初始化数据库管理器失败: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<soci::session> DbManager::getSession(const std::string &dataSourceName)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_initialized) {
        std::cerr << "数据库管理器未初始化" << std::endl;
        return nullptr;
    }

    auto it = m_connectionPools.find(dataSourceName);
    if (it == m_connectionPools.end()) {
        std::cerr << "未找到数据源: " << dataSourceName << std::endl;
        return nullptr;
    }

    auto &pool   = it->second;
    auto &config = m_configs[dataSourceName];

    // 如果池中有可用连接，返回第一个
    if (!pool.empty()) {
        auto session = pool.back();
        pool.pop_back();
        return session;
    }

    // 如果池已满，则创建一个新的连接但不添加到池中
    try {
        return std::make_shared<soci::session>(soci::odbc, config.connectionString);
    }
    catch (const std::exception &e) {
        std::cerr << "创建数据库连接失败: " << e.what() << std::endl;
        return nullptr;
    }
}

std::string DbManager::loadDsn(const std::string &dsnPath)
{
    std::ifstream file(dsnPath);
    if (!file.is_open()) {
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

DbManager::~DbManager()
{
    // 清理所有连接池
    for (auto &[name, pool]: m_connectionPools) {
        pool.clear();
    }
    m_connectionPools.clear();
}
