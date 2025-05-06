#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

class ApiHandler {
    public:
        static ApiHandler &instance();

        // 初始化API配置
        bool initialize(const std::string &apiConfigPath = "./config/api.json");

        // 处理API请求
        json handleRequest(const std::string &queryId, const json &params);

    private:
        ApiHandler() = default;

        // 禁用复制构造函数和赋值操作符
        ApiHandler(const ApiHandler &)            = delete;
        ApiHandler &operator=(const ApiHandler &) = delete;

        struct QueryInfo {
                std::string              id;
                std::string              dataSource;
                std::string              description;
                std::string              sql;
                std::vector<std::string> params;
        };

        std::map<std::string, QueryInfo> m_queries;
        bool                             m_initialized = false;
};
