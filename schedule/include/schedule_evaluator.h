#pragma once

#include "job_scheduler.h"
#include "schedule_chromosome.h"

namespace rtd {
namespace schedule {

/**
 * 评估派工方案的类
 */
class ScheduleEvaluator {
    public:
        /**
         * 构造函数
         * @param lotCount 批次数量
         * @param machineCount 机台数量
         * @param processingTimes 处理时间矩阵
         */
        ScheduleEvaluator(
          size_t                                  lotCount,
          size_t                                  machineCount,
          const std::vector<std::vector<double>> &processingTimes);

        /**
         * 评估染色体并返回适应度
         * @param chromosome 待评估的染色体
         * @return 适应度值(越大越好)
         */
        double evaluate(const Chromosome &chromosome) const;

        /**
         * 评估染色体并更新派工方案
         * @param chromosome 待评估的染色体
         * @param schedule 要更新的派工方案
         * @param lotIds 批次ID列表
         * @param machineIds 机台ID列表
         * @return 适应度值
         */
        double evaluateAndUpdate(
          const Chromosome               &chromosome,
          Schedule                       &schedule,
          const std::vector<std::string> &lotIds,
          const std::vector<std::string> &machineIds) const;

    private:
        size_t                           m_lotCount;
        size_t                           m_machineCount;
        std::vector<std::vector<double>> m_processingTimes;

        /**
         * 解码染色体为派工顺序列表
         * @param chromosome 染色体
         * @return 按机台分组的派工序列(机台索引->批次索引列表)
         */
        std::vector<std::vector<size_t>> decode(const Chromosome &chromosome) const;

        /**
         * 计算调度的完工时间
         * @param machineJobs 按机台分组的派工序列
         * @return 完工时间
         */
        double calculateMakespan(const std::vector<std::vector<size_t>> &machineJobs) const;

        /**
         * 计算每个批次的完工时间
         * @param machineJobs 按机台分组的派工序列
         * @return 每个批次的完工时间
         */
        std::vector<double> calculateCompletionTimes(
          const std::vector<std::vector<size_t>> &machineJobs) const;
};

}    // namespace schedule
}    // namespace rtd
