#pragma once

#include "../include/algorithm/archipelago_ga.hh"    // 修改引用路径
#include "job_scheduler.h"
#include "schedule_chromosome.h"
#include "schedule_evaluator.h"
#include <random>

namespace rtd {
namespace schedule {

/**
 * 基于多岛遗传算法的派工调度器实现
 */
class JobSchedulerImpl: public JobScheduler {
    public:
        JobSchedulerImpl();
        ~JobSchedulerImpl() override = default;

        // JobScheduler接口实现
        void                  setLots(const std::vector<std::string> &lotIds) override;
        void                  setMachines(const std::vector<std::string> &machineIds) override;
        bool                  setProcessingTimes(const std::vector<std::vector<double>> &processingTimes) override;
        bool                  setProcessingTime(size_t lotIndex, size_t machineIndex, double time) override;
        Schedule              calculateSchedule() override;
        std::future<Schedule> calculateScheduleAsync() override;

        // 遗传算法参数设置
        void setPopulationSize(size_t size) override { m_populationSize = size; }
        void setGenerationCount(size_t generations) override { m_generationCount = generations; }
        void setIslandCount(size_t islands) override { m_islandCount = islands; }
        void setCrossoverRate(double rate) override { m_crossoverRate = rate; }
        void setMutationRate(double rate) override { m_mutationRate = rate; }
        void setElitismCount(size_t count) override { m_elitismCount = count; }
        void setMigrationInterval(size_t interval) override { m_migrationInterval = interval; }
        void setMigrationRate(double rate) override { m_migrationRate = rate; }

    private:
        // 批次和机台信息
        std::vector<std::string>         m_lotIds;
        std::vector<std::string>         m_machineIds;
        std::vector<std::vector<double>> m_processingTimes;

        // GA参数
        size_t m_populationSize;
        size_t m_generationCount;
        size_t m_islandCount;
        double m_crossoverRate;
        double m_mutationRate;
        size_t m_elitismCount;
        size_t m_migrationInterval;
        double m_migrationRate;

        // 随机数生成
        std::mt19937 m_rng;

        // 实用方法
        Schedule decodeChromosome(const Chromosome &chromosome);
        bool     isValidProblem() const;
        void     validateInputs();

        // 多岛遗传算法实现
        class SchedulerGA;
};

}    // namespace schedule
}    // namespace rtd
