#pragma once

#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rtd {
namespace schedule {

/**
 * 调度数据管理器
 * 处理设备、批次和处理时间的数据访问
 */
class ScheduleDataManager {
    public:
        static std::unique_ptr<ScheduleDataManager> create();
        virtual ~ScheduleDataManager() = default;

        // 初始化数据管理器
        virtual bool initialize(const std::string &configPath = "./config") = 0;

        // 获取所有设备ID
        virtual std::vector<std::string> getAllEquipments() = 0;

        // 获取某个设备支持的所有批次ID
        virtual std::vector<std::string> getLotsByEquipment(const std::string &equipmentId) = 0;

        // 获取所有批次ID
        virtual std::vector<std::string> getAllLots() = 0;

        // 获取特定设备和批次的处理时间
        virtual double getProcessTime(const std::string &equipmentId, const std::string &lotId) = 0;

        // 获取处理时间矩阵
        virtual std::vector<std::vector<double>> getProcessTimeMatrix(
          const std::vector<std::string> &lots,
          const std::vector<std::string> &equipments) = 0;

        // 保存调度结果
        virtual bool saveDispatchResult(
          const std::string &equipmentId,
          const std::string &lotId,
          double             releaseTime,
          double             startTime,
          double             endTime) = 0;

        // 批量保存调度结果
        virtual bool saveDispatchResults(
          const std::vector<std::tuple<std::string, std::string, double, double, double>> &results) = 0;
};

}    // namespace schedule
}    // namespace rtd
