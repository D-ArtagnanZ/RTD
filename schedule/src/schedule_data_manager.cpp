#include "schedule_data_manager.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <set>    // 添加set头文件
#include <soci/odbc/soci-odbc.h>
#include <soci/soci.h>
#include <thread>
#include <unordered_map>

using json = nlohmann::json;
using namespace soci;

namespace rtd {
namespace schedule {

class ScheduleDataManagerImpl: public ScheduleDataManager {
    public:
        ScheduleDataManagerImpl() = default;
        ~ScheduleDataManagerImpl() override;

        bool                             initialize(const std::string &configPath) override;
        std::vector<std::string>         getAllEquipments() override;
        std::vector<std::string>         getLotsByEquipment(const std::string &equipmentId) override;
        std::vector<std::string>         getAllLots() override;
        double                           getProcessTime(const std::string &equipmentId, const std::string &lotId) override;
        std::vector<std::vector<double>> getProcessTimeMatrix(
          const std::vector<std::string> &lots,
          const std::vector<std::string> &equipments) override;
        bool saveDispatchResult(
          const std::string &equipmentId,
          const std::string &lotId,
          double             releaseTime,
          double             startTime,
          double             endTime) override;
        bool saveDispatchResults(
          const std::vector<std::tuple<std::string, std::string, double, double, double>> &results) override;

    private:
        // 数据源配置
        struct DataSourceConfig {
                std::string dsnPath;
                int         minConn;
                int         maxConn;
                int         connTimeout;
                std::string connectionString;
        };

        // API查询配置
        struct QueryInfo {
                std::string              id;
                std::string              dataSource;
                std::string              description;
                std::string              sql;
                std::vector<std::string> params;
        };

        std::map<std::string, DataSourceConfig>         m_configs;
        std::map<std::string, std::shared_ptr<session>> m_sessions;
        std::map<std::string, QueryInfo>                m_queries;
        std::mutex                                      m_mutex;
        bool                                            m_initialized = false;

        // 加载配置文件
        bool loadDataSourceConfig(const std::string &filePath);
        bool loadApiConfig(const std::string &filePath);

        // 加载DSN文件
        std::string loadDsn(const std::string &dsnPath);

        // 获取数据库会话
        std::shared_ptr<session> getSession(const std::string &dataSourceName);
};

ScheduleDataManagerImpl::~ScheduleDataManagerImpl()
{
    // 清理会话
    m_sessions.clear();
}

bool ScheduleDataManagerImpl::initialize(const std::string &configPath)
{
    if (m_initialized) {
        return true;
    }

    try {
        // 加载配置文件
        if (!loadDataSourceConfig(configPath + "/data_source.json")) {
            std::cerr << "Failed to load data source configuration" << std::endl;
            return false;
        }

        if (!loadApiConfig(configPath + "/api.json")) {
            std::cerr << "Failed to load API configuration" << std::endl;
            return false;
        }

        // 初始化数据库连接
        for (const auto &[name, config]: m_configs) {
            try {
                auto sess        = std::make_shared<session>(odbc, config.connectionString);
                m_sessions[name] = sess;
                std::cout << "Connected to " << name << " database" << std::endl;
            }
            catch (const std::exception &e) {
                std::cerr << "Failed to connect to " << name << ": " << e.what() << std::endl;
                return false;
            }
        }

        m_initialized = true;
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
        return false;
    }
}

bool ScheduleDataManagerImpl::loadDataSourceConfig(const std::string &filePath)
{
    try {
        std::ifstream configFile(filePath);
        if (!configFile.is_open()) {
            std::cerr << "Cannot open data source config file: " << filePath << std::endl;
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
                std::cerr << "Failed to load DSN file: " << sourceConfig.dsnPath << std::endl;
                continue;
            }

            m_configs[name] = sourceConfig;
        }

        return !m_configs.empty();
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to load data source config: " << e.what() << std::endl;
        return false;
    }
}

bool ScheduleDataManagerImpl::loadApiConfig(const std::string &filePath)
{
    try {
        std::ifstream configFile(filePath);
        if (!configFile.is_open()) {
            std::cerr << "Cannot open API config file: " << filePath << std::endl;
            return false;
        }

        json config;
        configFile >> config;

        // 解析查询配置
        for (const auto &query: config["api"]) {
            QueryInfo info;
            info.id          = query["id"];
            info.dataSource  = query["data_source"];
            info.description = query["description"];
            info.sql         = query["sql"];

            for (const auto &param: query["params"]) {
                info.params.push_back(param);
            }

            m_queries[info.id] = info;
        }

        return !m_queries.empty();
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to load API config: " << e.what() << std::endl;
        return false;
    }
}

std::string ScheduleDataManagerImpl::loadDsn(const std::string &dsnPath)
{
    std::ifstream file(dsnPath);
    if (!file.is_open()) {
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

std::shared_ptr<session> ScheduleDataManagerImpl::getSession(const std::string &dataSourceName)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_sessions.find(dataSourceName);
    if (it != m_sessions.end()) {
        return it->second;
    }

    // 如果会话不存在，尝试创建一个新的
    auto configIt = m_configs.find(dataSourceName);
    if (configIt != m_configs.end()) {
        try {
            auto sess                  = std::make_shared<session>(odbc, configIt->second.connectionString);
            m_sessions[dataSourceName] = sess;
            return sess;
        }
        catch (const std::exception &e) {
            std::cerr << "Failed to create database session: " << e.what() << std::endl;
        }
    }

    return nullptr;
}

std::vector<std::string> ScheduleDataManagerImpl::getAllEquipments()
{
    std::vector<std::string> equipments;

    try {
        auto it = m_queries.find("getEqpList");
        if (it == m_queries.end()) {
            throw std::runtime_error("Query not found: getEqpList");
        }

        const QueryInfo &query   = it->second;
        auto             session = getSession(query.dataSource);
        if (!session) {
            throw std::runtime_error("Failed to get database session");
        }

        rowset<row> rs = (session->prepare << query.sql);

        for (const auto &r: rs) {
            equipments.push_back(r.get<std::string>("EQP_ID"));
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to get equipments: " << e.what() << std::endl;
    }

    return equipments;
}

std::vector<std::string> ScheduleDataManagerImpl::getLotsByEquipment(const std::string &equipmentId)
{
    std::vector<std::string> lots;

    try {
        auto it = m_queries.find("getLotListByEqpId");
        if (it == m_queries.end()) {
            throw std::runtime_error("Query not found: getLotListByEqpId");
        }

        const QueryInfo &query   = it->second;
        auto             session = getSession(query.dataSource);
        if (!session) {
            throw std::runtime_error("Failed to get database session");
        }

        // 修复: 使用正确方式创建有参数的rowset
        rowset<row> rs = (session->prepare << query.sql, use(equipmentId));

        for (const auto &r: rs) {
            lots.push_back(r.get<std::string>("LOT_ID"));
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to get lots by equipment: " << e.what() << std::endl;
    }

    return lots;
}

std::vector<std::string> ScheduleDataManagerImpl::getAllLots()
{
    std::vector<std::string> lots;
    std::set<std::string>    uniqueLots;    // 使用集合去重

    try {
        // 获取所有设备
        auto equipments = getAllEquipments();

        // 对每个设备获取批次，并去重
        for (const auto &eqp: equipments) {
            auto eqpLots = getLotsByEquipment(eqp);
            for (const auto &lot: eqpLots) {
                uniqueLots.insert(lot);
            }
        }

        lots.assign(uniqueLots.begin(), uniqueLots.end());
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to get all lots: " << e.what() << std::endl;
    }

    return lots;
}

double ScheduleDataManagerImpl::getProcessTime(const std::string &equipmentId, const std::string &lotId)
{
    try {
        auto it = m_queries.find("getProcessTime");
        if (it == m_queries.end()) {
            throw std::runtime_error("Query not found: getProcessTime");
        }

        const QueryInfo &query   = it->second;
        auto             session = getSession(query.dataSource);
        if (!session) {
            throw std::runtime_error("Failed to get database session");
        }

        double    processTime = 0.0;
        indicator ind;

        // 使用正确的SOCI API方式执行参数化查询并获取结果
        *session << query.sql, use(equipmentId), use(lotId), into(processTime, ind);

        if (ind == i_null) {
            return 0.0;    // 如果结果为NULL，返回0
        }

        return processTime;
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to get process time: " << e.what() << std::endl;
        return 0.0;
    }
}

std::vector<std::vector<double>> ScheduleDataManagerImpl::getProcessTimeMatrix(
  const std::vector<std::string> &lots,
  const std::vector<std::string> &equipments)
{
    std::vector<std::vector<double>> matrix(lots.size(), std::vector<double>(equipments.size(), 0.0));

    // 创建批次ID到索引的映射
    std::unordered_map<std::string, size_t> lotIndexMap;
    for (size_t i = 0; i < lots.size(); ++i) {
        lotIndexMap[lots[i]] = i;
    }

    // 创建设备ID到索引的映射
    std::unordered_map<std::string, size_t> equipmentIndexMap;
    for (size_t i = 0; i < equipments.size(); ++i) {
        equipmentIndexMap[equipments[i]] = i;
    }

    try {
        // 一次性获取所有工艺兼容性数据
        auto session = getSession("Oracle");    // 假设兼容性数据在Oracle数据库
        if (!session) {
            throw std::runtime_error("Failed to get database session");
        }

        for (size_t i = 0; i < lots.size(); ++i) {
            for (size_t j = 0; j < equipments.size(); ++j) {
                matrix[i][j] = getProcessTime(equipments[j], lots[i]);
            }
        }

        // // 修复: 使用正确方式创建rowset
        // rowset<row> rs = (session->prepare << "SELECT LOT_ID, EQP_ID, PROCESS_TIME FROM PROCESS_COMPATIBILITY WHERE PROCESS_TIME > 0");

        // // 填充处理时间矩阵
        // for (const auto &r: rs) {
        //     std::string lotId       = r.get<std::string>("LOT_ID");
        //     std::string eqpId       = r.get<std::string>("EQP_ID");
        //     double      processTime = r.get<double>("PROCESS_TIME");

        //     auto lotIt = lotIndexMap.find(lotId);
        //     auto eqpIt = equipmentIndexMap.find(eqpId);

        //     if (lotIt != lotIndexMap.end() && eqpIt != equipmentIndexMap.end()) {
        //         matrix[lotIt->second][eqpIt->second] = processTime;
        //     }
        // }
    }
    catch (const std::exception &e) {
        std::cerr << "获取处理时间矩阵出错: " << e.what() << std::endl;
        // std::cerr << "将使用单独查询方式获取" << std::endl;

        // 回退到单独查询每个处理时间的方式
        // for (size_t i = 0; i < lots.size(); ++i) {
        //     for (size_t j = 0; j < equipments.size(); ++j) {
        //         matrix[i][j] = getProcessTime(equipments[j], lots[i]);
        //     }
        // }
    }

    return matrix;
}

bool ScheduleDataManagerImpl::saveDispatchResult(
  const std::string &equipmentId,
  const std::string &lotId,
  double             releaseTime,
  double             startTime,
  double             endTime)
{
    try {
        // 获取查询信息
        auto it = m_queries.find("insertDispatch");
        if (it == m_queries.end()) {
            std::cerr << "Query not found: insertDispatch" << std::endl;
            return false;
        }

        const QueryInfo &query   = it->second;
        auto             session = getSession(query.dataSource);
        if (!session) {
            std::cerr << "Failed to get database session for " << query.dataSource << std::endl;
            return false;
        }

        // 正确使用SOCI参数绑定语法
        *session << query.sql,
          use(equipmentId), use(lotId), use(releaseTime),
          use(startTime), use(endTime);

        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "Failed to save dispatch result: " << e.what() << std::endl;
        return false;
    }
}

bool ScheduleDataManagerImpl::saveDispatchResults(
  const std::vector<std::tuple<std::string, std::string, double, double, double>> &results)
{
    bool success = true;

    // 获取查询信息提前
    auto it = m_queries.find("insertDispatch");
    if (it == m_queries.end()) {
        std::cerr << "Query not found: insertDispatch" << std::endl;
        return false;
    }

    const QueryInfo &query   = it->second;
    auto             session = getSession(query.dataSource);
    if (!session) {
        std::cerr << "Failed to get database session for " << query.dataSource << std::endl;
        return false;
    }

    try {
        // 优化：准备一次语句，多次执行
        statement stmt = (session->prepare << query.sql);

        for (const auto &[eqpId, lotId, releaseTime, startTime, endTime]: results) {
            try {
                // 绑定参数并执行
                stmt.exchange(use(eqpId));
                stmt.exchange(use(lotId));
                stmt.exchange(use(releaseTime));
                stmt.exchange(use(startTime));
                stmt.exchange(use(endTime));

                stmt.define_and_bind();
                stmt.execute(true);
            }
            catch (const std::exception &e) {
                std::cerr << "Error executing statement: " << e.what() << std::endl;
                success = false;
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Batch insert failed: " << e.what() << std::endl;
        success = false;
    }

    return success;
}

std::unique_ptr<ScheduleDataManager> ScheduleDataManager::create()
{
    return std::make_unique<ScheduleDataManagerImpl>();
}

}    // namespace schedule
}    // namespace rtd
