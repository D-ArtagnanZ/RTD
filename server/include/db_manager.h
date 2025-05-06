#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <soci/soci.h>
#include <string>

using json = nlohmann::json;

class DbManager {
    public:
        static DbManager &instance();

        // 初始化数据库连接池
        bool initialize(const std::string &configPath = "./config/data_source.json");

        // 获取数据库会话
        std::shared_ptr<soci::session> getSession(const std::string &dataSourceName);

    private:
        DbManager() = default;
        ~DbManager();

        // 禁用复制构造函数和赋值操作符
        DbManager(const DbManager &)            = delete;
        DbManager &operator=(const DbManager &) = delete;

        // 加载DSN配置
        std::string loadDsn(const std::string &dsnPath);

        // 数据源配置
        struct DataSourceConfig {
                std::string dsnPath;
                int         minConn;
                int         maxConn;
                int         connTimeout;
                std::string connectionString;
        };

        std::map<std::string, DataSourceConfig>                            m_configs;
        std::map<std::string, std::vector<std::shared_ptr<soci::session>>> m_connectionPools;
        std::mutex                                                         m_mutex;
        bool                                                               m_initialized = false;
};
