#include "api_handler.h"
#include "db_manager.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <soci/row.h>
#include <soci/rowset.h>
#include <soci/soci.h>
#include <soci/values.h>
#include <sstream>

ApiHandler &ApiHandler::instance()
{
    static ApiHandler instance;
    return instance;
}

bool ApiHandler::initialize(const std::string &apiConfigPath)
{
    if (m_initialized) {
        return true;
    }

    try {
        // 读取API配置文件
        std::ifstream configFile(apiConfigPath);
        if (!configFile.is_open()) {
            std::cerr << "无法打开API配置文件: " << apiConfigPath << std::endl;
            return false;
        }

        json config;
        configFile >> config;

        // 解析查询配置
        for (const auto &query: config["queries"]) {
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

        m_initialized = true;
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "初始化API处理器失败: " << e.what() << std::endl;
        return false;
    }
}

json ApiHandler::handleRequest(const std::string &queryId, const json &params)
{
    if (!m_initialized) {
        return {
          {"error", "API处理器未初始化"}
        };
    }

    auto it = m_queries.find(queryId);
    if (it == m_queries.end()) {
        return {
          {"error", "未找到查询: " + queryId}
        };
    }

    const auto &query = it->second;

    // 获取数据库会话
    auto session = DbManager::instance().getSession(query.dataSource);
    if (!session) {
        return {
          {"error", "无法获取数据库连接"}
        };
    }

    try {
        // 准备SQL语句
        soci::statement stmt = session->prepare << query.sql;

        // 临时保存参数值用于绑定
        std::string strValue;
        int         intValue;
        double      doubleValue;

        // 绑定参数
        for (const auto &paramName: query.params) {
            if (!params.contains(paramName)) {
                return {
                  {"error", "缺少参数: " + paramName}
                };
            }

            const auto &paramValue = params[paramName];

            if (paramValue.is_string()) {
                strValue = paramValue.get<std::string>();
                stmt.exchange(soci::use(strValue));
            }
            else if (paramValue.is_number_integer()) {
                intValue = paramValue.get<int>();
                stmt.exchange(soci::use(intValue));
            }
            else if (paramValue.is_number_float()) {
                doubleValue = paramValue.get<double>();
                stmt.exchange(soci::use(doubleValue));
            }
            else {
                return {
                  {"error", "不支持的参数类型: " + paramName}
                };
            }
        }

        // 执行查询
        soci::row row;
        stmt.exchange(soci::into(row));    // 使用into()绑定row对象
        stmt.define_and_bind();
        stmt.execute();

        // 构建结果
        json result = json::array();
        while (stmt.fetch()) {    // 无需传递参数
            json rowData;
            for (size_t i = 0; i < row.size(); ++i) {
                const soci::column_properties &props      = row.get_properties(i);
                std::string                    columnName = props.get_name();

                if (row.get_indicator(i) == soci::i_null) {
                    rowData[columnName] = nullptr;
                    continue;
                }

                switch (props.get_data_type()) {
                    case soci::dt_string:
                        rowData[columnName] = row.get<std::string>(i);
                        break;
                    case soci::dt_double:
                        rowData[columnName] = row.get<double>(i);
                        break;
                    case soci::dt_integer:
                        rowData[columnName] = row.get<int>(i);
                        break;
                    case soci::dt_long_long:
                        rowData[columnName] = row.get<long long>(i);
                        break;
                    case soci::dt_unsigned_long_long:
                        rowData[columnName] = row.get<unsigned long long>(i);
                        break;
                    case soci::dt_date:
                        {
                            // 将tm结构转换为字符串
                            std::tm            time = row.get<std::tm>(i);
                            std::ostringstream ss;
                            ss << std::put_time(&time, "%Y-%m-%d %H:%M:%S");
                            rowData[columnName] = ss.str();
                            break;
                        }
                    default:
                        rowData[columnName] = row.get<std::string>(i);
                }
            }
            result.push_back(rowData);
        }

        return {
          {"data", result}
        };
    }
    catch (const std::exception &e) {
        return {
          {"error", std::string("查询执行失败: ") + e.what()}
        };
    }
}
