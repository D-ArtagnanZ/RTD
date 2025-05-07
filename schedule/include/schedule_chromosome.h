#pragma once

#include <algorithm>
#include <random>
#include <stdexcept>
#include <vector>

namespace rtd {
namespace schedule {

/**
 * 代表一个派工方案编码的染色体
 */
class Chromosome {
    public:
        /**
         * 创建空染色体
         */
        Chromosome() = default;

        /**
         * 创建指定基因长度的染色体
         */
        explicit Chromosome(size_t length)
            : m_genes(length, 0) {}

        /**
         * 使用给定基因序列创建染色体
         */
        explicit Chromosome(const std::vector<size_t> &genes)
            : m_genes(genes) {}

        /**
         * 获取基因值
         */
        size_t getGene(size_t index) const
        {
            if (index >= m_genes.size()) {
                throw std::out_of_range("Gene index out of range");
            }
            return m_genes[index];
        }

        /**
         * 设置基因值
         */
        void setGene(size_t index, size_t value)
        {
            if (index >= m_genes.size()) {
                throw std::out_of_range("Gene index out of range");
            }
            m_genes[index] = value;
        }

        /**
         * 获取染色体长度
         */
        size_t getLength() const
        {
            return m_genes.size();
        }

        /**
         * 获取基因序列
         */
        const std::vector<size_t> &getGenes() const
        {
            return m_genes;
        }

        /**
         * 创建一个随机染色体
         * @param lotCount 批次数量
         * @param machineCount 机台数量
         * @param processingTimes 处理时间矩阵
         * @param generator 随机数生成器
         * @return 一个有效的随机染色体
         */
        static Chromosome createRandom(
          size_t                                  lotCount,
          size_t                                  machineCount,
          const std::vector<std::vector<double>> &processingTimes,
          std::mt19937                           &generator);

        /**
         * 交叉操作 - 使用顺序交叉(OX)
         * @param other 另一个父染色体
         * @param generator 随机数生成器
         * @return 子代染色体
         */
        Chromosome crossover(
          const Chromosome &other,
          std::mt19937     &generator) const;

        /**
         * 变异操作 - 使用交换变异
         * @param mutationRate 变异率
         * @param generator 随机数生成器
         */
        void mutate(double mutationRate, std::mt19937 &generator);

        /**
         * 检查染色体是否有效
         * @param lotCount 批次数量
         * @param machineCount 机台数量
         * @param processingTimes 处理时间矩阵
         * @return 是否有效
         */
        bool isValid(
          size_t                                  lotCount,
          size_t                                  machineCount,
          const std::vector<std::vector<double>> &processingTimes) const;

        /**
         * 修复无效染色体
         * @param lotCount 批次数量
         * @param machineCount 机台数量
         * @param processingTimes 处理时间矩阵
         * @param generator 随机数生成器
         */
        void repair(
          size_t                                  lotCount,
          size_t                                  machineCount,
          const std::vector<std::vector<double>> &processingTimes,
          std::mt19937                           &generator);

    private:
        std::vector<size_t> m_genes;
};

}    // namespace schedule
}    // namespace rtd
