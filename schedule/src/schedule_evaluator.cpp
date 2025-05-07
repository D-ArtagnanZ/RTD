#include "schedule_evaluator.h"
#include <algorithm>
#include <numeric>

namespace rtd {
namespace schedule {

ScheduleEvaluator::ScheduleEvaluator(
  size_t                                  lotCount,
  size_t                                  machineCount,
  const std::vector<std::vector<double>> &processingTimes)
    : m_lotCount(lotCount), m_machineCount(machineCount), m_processingTimes(processingTimes)
{}

double ScheduleEvaluator::evaluate(const Chromosome &chromosome) const
{
    // 将染色体解码为机台作业序列
    std::vector<std::vector<size_t>> machineJobs = decode(chromosome);

    // 计算评价指标
    double makespan = calculateMakespan(machineJobs);

    // 返回适应度值（负值，因为我们要最小化完工时间）
    return -makespan;
}

double ScheduleEvaluator::evaluateAndUpdate(
  const Chromosome               &chromosome,
  Schedule                       &schedule,
  const std::vector<std::string> &lotIds,
  const std::vector<std::string> &machineIds) const
{
    // 清空原有调度
    schedule.clear();

    // 将染色体解码为机台作业序列
    std::vector<std::vector<size_t>> machineJobs = decode(chromosome);

    // 计算每个批次的开始和结束时间
    std::vector<double> machineEndTimes(m_machineCount, 0.0);

    // 构建调度结果
    for (size_t machineIdx = 0; machineIdx < machineJobs.size(); ++machineIdx) {
        double currentTime = 0.0;

        for (size_t lotIdx: machineJobs[machineIdx]) {
            // 获取处理时间
            double processTime = m_processingTimes[lotIdx][machineIdx];

            // 只处理有效的处理时间
            if (processTime > 0) {
                // 设置开始和结束时间
                double startTime = currentTime;
                double endTime   = startTime + processTime;

                // 更新当前时间
                currentTime = endTime;

                // 添加派工分配
                JobAssignment assignment(
                  lotIdx,
                  lotIdx < lotIds.size() ? lotIds[lotIdx] : "Unknown",
                  machineIdx,
                  machineIdx < machineIds.size() ? machineIds[machineIdx] : "Unknown",
                  processTime,
                  startTime,
                  endTime);

                schedule.addAssignment(assignment);

                // 更新机台结束时间
                machineEndTimes[machineIdx] = endTime;
            }
        }
    }

    // 计算评价指标
    schedule.makespan = *std::max_element(machineEndTimes.begin(), machineEndTimes.end());

    // 计算平均流通时间
    double totalFlowTime = 0.0;
    for (const auto &assignment: schedule.assignments) {
        totalFlowTime += assignment.endTime;
    }
    schedule.meanFlowTime = totalFlowTime / schedule.assignments.size();

    // 计算最大延迟（这里简单假设没有截止时间，所以为0）
    schedule.maxTardiness = 0.0;

    // 返回适应度值（负值，因为我们要最小化完工时间）
    return -schedule.makespan;
}

std::vector<std::vector<size_t>> ScheduleEvaluator::decode(const Chromosome &chromosome) const
{
    // 初始化机台作业序列
    std::vector<std::vector<size_t>> machineJobs(m_machineCount);

    // 解码染色体
    for (size_t i = 0; i < chromosome.getLength(); ++i) {
        size_t gene         = chromosome.getGene(i);
        size_t lotIndex     = gene / m_machineCount;
        size_t machineIndex = gene % m_machineCount;

        // 检查索引是否有效
        if (lotIndex < m_lotCount && machineIndex < m_machineCount) {
            // 检查处理时间是否有效 (大于0)
            if (m_processingTimes[lotIndex][machineIndex] > 0) {
                machineJobs[machineIndex].push_back(lotIndex);
            }
        }
    }

    return machineJobs;
}

double ScheduleEvaluator::calculateMakespan(const std::vector<std::vector<size_t>> &machineJobs) const
{
    std::vector<double> machineEndTimes(m_machineCount, 0.0);

    // 计算每个机台的完工时间
    for (size_t machineIdx = 0; machineIdx < machineJobs.size(); ++machineIdx) {
        double currentTime = 0.0;

        for (size_t lotIdx: machineJobs[machineIdx]) {
            // 获取处理时间
            double processTime = m_processingTimes[lotIdx][machineIdx];

            // 更新当前时间
            currentTime += processTime;
        }

        // 更新机台结束时间
        machineEndTimes[machineIdx] = currentTime;
    }

    // 返回最大完工时间
    return *std::max_element(machineEndTimes.begin(), machineEndTimes.end());
}

std::vector<double> ScheduleEvaluator::calculateCompletionTimes(
  const std::vector<std::vector<size_t>> &machineJobs) const
{
    std::vector<double> completionTimes(m_lotCount, 0.0);

    // 计算每个批次的完工时间
    for (size_t machineIdx = 0; machineIdx < machineJobs.size(); ++machineIdx) {
        double currentTime = 0.0;

        for (size_t lotIdx: machineJobs[machineIdx]) {
            // 获取处理时间
            double processTime = m_processingTimes[lotIdx][machineIdx];

            // 更新当前时间
            currentTime += processTime;

            // 记录完工时间
            completionTimes[lotIdx] = currentTime;
        }
    }

    return completionTimes;
}

}    // namespace schedule
}    // namespace rtd
