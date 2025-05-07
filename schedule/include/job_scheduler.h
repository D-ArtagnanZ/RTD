#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace rtd {
namespace schedule {

/**
 * 派工结果项
 * 表示一个批次在特定机台上的调度
 */
struct JobAssignment {
        size_t      lotIndex;          // 批次索引
        std::string lotId;             // 批次ID
        size_t      machineIndex;      // 机台索引
        std::string machineId;         // 机台ID
        double      processingTime;    // 处理时间
        double      startTime;         // 开始时间
        double      endTime;           // 结束时间

        JobAssignment(size_t lotIdx, const std::string &lot, size_t machIdx, const std::string &mach, double procTime, double start = 0, double end = 0)
            : lotIndex(lotIdx), lotId(lot), machineIndex(machIdx), machineId(mach), processingTime(procTime), startTime(start), endTime(end) {}
};

/**
 * 派工方案
 * 包含所有批次的分配方案和评价指标
 */
struct Schedule {
        std::vector<JobAssignment>              assignments;           // 所有派工分配
        std::vector<std::vector<JobAssignment>> machineAssignments;    // 按机台分组的派工
        double                                  makespan;              // 总完工时间
        double                                  meanFlowTime;          // 平均流通时间
        double                                  maxTardiness;          // 最大延迟

        // 添加一个派工结果
        void addAssignment(const JobAssignment &assignment)
        {
            assignments.push_back(assignment);

            // 如果机台索引超出当前大小，扩展机台分配数组
            if (assignment.machineIndex >= machineAssignments.size()) {
                machineAssignments.resize(assignment.machineIndex + 1);
            }
            machineAssignments[assignment.machineIndex].push_back(assignment);
        }

        // 清空所有派工结果
        void clear()
        {
            assignments.clear();
            machineAssignments.clear();
            makespan     = 0;
            meanFlowTime = 0;
            maxTardiness = 0;
        }
};

/**
 * 派工调度器接口
 * 用于计算最优派工方案
 */
class JobScheduler {
    public:
        virtual ~JobScheduler() = default;

        /**
         * 设置批次ID列表
         */
        virtual void setLots(const std::vector<std::string> &lotIds) = 0;

        /**
         * 设置机台ID列表
         */
        virtual void setMachines(const std::vector<std::string> &machineIds) = 0;

        /**
         * 设置处理时间矩阵
         * @param processingTimes m×n的矩阵，processingTimes[i][j]表示批次i在机台j上的处理时间
         * @return 是否成功设置
         */
        virtual bool setProcessingTimes(const std::vector<std::vector<double>> &processingTimes) = 0;

        /**
         * 设置单个处理时间
         * @param lotIndex 批次索引
         * @param machineIndex 机台索引
         * @param time 处理时间
         * @return 是否成功设置
         */
        virtual bool setProcessingTime(size_t lotIndex, size_t machineIndex, double time) = 0;

        /**
         * 计算最优派工方案
         * @return 派工方案
         */
        virtual Schedule calculateSchedule() = 0;

        /**
         * 异步计算派工方案
         * @return 表示将来派工方案的future
         */
        virtual std::future<Schedule> calculateScheduleAsync() = 0;

        // 添加调度参数设置方法
        virtual void setPopulationSize(size_t size)         = 0;
        virtual void setGenerationCount(size_t generations) = 0;
        virtual void setIslandCount(size_t islands)         = 0;
        virtual void setCrossoverRate(double rate)          = 0;
        virtual void setMutationRate(double rate)           = 0;
        virtual void setElitismCount(size_t count)          = 0;
        virtual void setMigrationInterval(size_t interval)  = 0;
        virtual void setMigrationRate(double rate)          = 0;

        /**
         * 创建新的派工调度器实例
         */
        static std::unique_ptr<JobScheduler> create();
};

}    // namespace schedule
}    // namespace rtd
