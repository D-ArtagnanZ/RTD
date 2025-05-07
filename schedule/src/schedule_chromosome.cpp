#include "schedule_chromosome.h"
#include <numeric>
#include <unordered_set>

namespace rtd {
namespace schedule {

// 创建随机染色体时确保所有批次分配到有效机台
Chromosome Chromosome::createRandom(
  size_t                                  lotCount,
  size_t                                  machineCount,
  const std::vector<std::vector<double>> &processingTimes,
  std::mt19937                           &generator)
{
    // 创建一个有效的分配序列
    std::vector<size_t> validGenes;

    // 对于每个批次，找到所有有效的机台分配
    for (size_t i = 0; i < lotCount; ++i) {
        std::vector<size_t> validMachines;
        for (size_t j = 0; j < machineCount; ++j) {
            // 仅当处理时间大于零时才是有效分配
            if (processingTimes[i][j] > 0) {
                // 有效分配的编码：i * machineCount + j
                validMachines.push_back(i * machineCount + j);
            }
        }

        // 如果有有效机台，随机选一个加入序列
        if (!validMachines.empty()) {
            std::uniform_int_distribution<size_t> dist(0, validMachines.size() - 1);
            validGenes.push_back(validMachines[dist(generator)]);
        }
    }

    // 随机打乱序列顺序
    std::shuffle(validGenes.begin(), validGenes.end(), generator);

    return Chromosome(validGenes);
}

Chromosome Chromosome::crossover(const Chromosome &other, std::mt19937 &generator) const
{
    if (m_genes.size() != other.m_genes.size()) {
        throw std::invalid_argument("Chromosomes must have the same length");
    }

    const size_t length = m_genes.size();
    if (length <= 2) {
        return *this;    // 太短无法交叉，直接返回
    }

    // 使用顺序交叉(OX)
    std::uniform_int_distribution<size_t> dist(0, length - 1);
    size_t                                start = dist(generator);
    size_t                                end   = dist(generator);

    // 确保start < end
    if (start > end) {
        std::swap(start, end);
    }

    // 创建子代
    std::vector<size_t>        childGenes(length, 0);
    std::unordered_set<size_t> usedGenes;

    // 将父代的中间部分复制到子代
    for (size_t i = start; i <= end; ++i) {
        childGenes[i] = m_genes[i];
        usedGenes.insert(m_genes[i]);
    }

    // 从另一个父代中按顺序取未使用的基因填充
    size_t j = (end + 1) % length;
    for (size_t i = 0; i < length; ++i) {
        size_t pos = (end + 1 + i) % length;
        if (usedGenes.find(other.m_genes[pos]) == usedGenes.end()) {
            childGenes[j] = other.m_genes[pos];
            usedGenes.insert(other.m_genes[pos]);
            j = (j + 1) % length;
        }
    }

    return Chromosome(childGenes);
}

void Chromosome::mutate(double mutationRate, std::mt19937 &generator)
{
    if (m_genes.size() <= 1) {
        return;    // 太短无法变异
    }

    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // 对每个基因位置尝试变异
    for (size_t i = 0; i < m_genes.size() - 1; ++i) {
        if (dist(generator) < mutationRate) {
            // 与一个随机位置交换
            std::uniform_int_distribution<size_t> posDist(0, m_genes.size() - 1);
            size_t                                j = posDist(generator);
            std::swap(m_genes[i], m_genes[j]);
        }
    }
}

// 修改染色体验证方法，确保只考虑有效的处理时间
bool Chromosome::isValid(
  size_t                                  lotCount,
  size_t                                  machineCount,
  const std::vector<std::vector<double>> &processingTimes) const
{
    // 检查每个批次是否只出现一次
    std::vector<bool> lotUsed(lotCount, false);

    for (size_t gene: m_genes) {
        size_t lot     = gene / machineCount;
        size_t machine = gene % machineCount;

        // 批次索引越界
        if (lot >= lotCount) {
            return false;
        }

        // 机台索引越界
        if (machine >= machineCount) {
            return false;
        }

        // 批次重复出现
        if (lotUsed[lot]) {
            return false;
        }

        // 处理时间无效 - 必须大于零才是有效配对
        if (processingTimes[lot][machine] <= 0) {
            return false;
        }

        lotUsed[lot] = true;
    }

    // 检查是否所有批次都被分配
    // 注：这里我们不再强制要求所有批次都必须分配，因为有些批次可能没有有效的机台

    return true;
}

// 修改染色体修复方法，确保只分配到有效的机台
void Chromosome::repair(
  size_t                                  lotCount,
  size_t                                  machineCount,
  const std::vector<std::vector<double>> &processingTimes,
  std::mt19937                           &generator)
{
    // 检查每个批次是否分配到有效机台
    std::vector<bool>   lotAssigned(lotCount, false);
    std::vector<size_t> invalidPositions;

    for (size_t i = 0; i < m_genes.size(); ++i) {
        size_t gene    = m_genes[i];
        size_t lot     = gene / machineCount;
        size_t machine = gene % machineCount;

        // 检查基因是否有效
        bool valid = true;

        // 批次索引越界
        if (lot >= lotCount) {
            valid = false;
        }
        // 机台索引越界
        else if (machine >= machineCount) {
            valid = false;
        }
        // 处理时间无效
        else if (processingTimes[lot][machine] <= 0) {
            valid = false;
        }
        // 批次重复
        else if (lotAssigned[lot]) {
            valid = false;
        }

        if (valid) {
            lotAssigned[lot] = true;
        }
        else {
            invalidPositions.push_back(i);
        }
    }

    // 找出未分配的批次
    std::vector<size_t> unassignedLots;
    for (size_t i = 0; i < lotCount; ++i) {
        if (!lotAssigned[i]) {
            unassignedLots.push_back(i);
        }
    }

    // 处理无效位置和未分配批次
    for (size_t i = 0; i < invalidPositions.size() || !unassignedLots.empty();) {
        if (i < invalidPositions.size()) {
            size_t pos = invalidPositions[i];

            if (!unassignedLots.empty()) {
                // 修复为未分配批次的有效机台
                size_t lot = unassignedLots.back();
                unassignedLots.pop_back();

                // 找到该批次的有效机台
                std::vector<size_t> validMachines;
                for (size_t j = 0; j < machineCount; ++j) {
                    if (processingTimes[lot][j] > 0) {    // 确保处理时间大于0
                        validMachines.push_back(j);
                    }
                }

                if (!validMachines.empty()) {
                    std::uniform_int_distribution<size_t> dist(0, validMachines.size() - 1);
                    size_t                                machine = validMachines[dist(generator)];
                    m_genes[pos]                                  = lot * machineCount + machine;
                }

                ++i;
            }
            else {
                // 删除多余的基因
                m_genes.erase(m_genes.begin() + pos);
                invalidPositions.erase(invalidPositions.begin() + i);
                // 不增加i，因为数组已经移除一个元素
            }
        }
        else if (!unassignedLots.empty()) {
            // 添加新基因
            size_t lot = unassignedLots.back();
            unassignedLots.pop_back();

            // 找到该批次的有效机台
            std::vector<size_t> validMachines;
            for (size_t j = 0; j < machineCount; ++j) {
                if (processingTimes[lot][j] > 0) {    // 确保处理时间大于0
                    validMachines.push_back(j);
                }
            }

            if (!validMachines.empty()) {
                std::uniform_int_distribution<size_t> dist(0, validMachines.size() - 1);
                size_t                                machine = validMachines[dist(generator)];
                m_genes.push_back(lot * machineCount + machine);
            }
        }
    }
}

}    // namespace schedule
}    // namespace rtd
