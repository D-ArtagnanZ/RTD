#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

namespace rtd {
namespace algorithm {

/**
 * 迁移策略 - 定义如何选择迁移个体
 */
enum class MigrationPolicy {
    BEST,             // 迁移最优个体
    RANDOM,           // 随机选择个体迁移
    TOURNAMENT,       // 锦标赛选择迁移个体
    ROULETTE_WHEEL    // 轮盘赌选择迁移个体
};

/**
 * 迁移拓扑 - 定义岛屿之间的连接结构
 */
enum class MigrationTopology {
    RING,               // 环形连接（每个岛屿连接到相邻岛屿）
    FULLY_CONNECTED,    // 全连接（任意两个岛屿之间都有连接）
    STAR,               // 星形连接（一个中心岛屿连接到所有其他岛屿）
    MESH                // 网格连接（岛屿按网格排列，连接到邻居）
};

/**
 * 多岛遗传算法抽象基类
 * 将种群分成多个隔离的"岛屿"，每个岛屿独立演化，并周期性地交换个体
 */
template<typename Genotype, typename Phenotype, typename FitnessType>
class ArchipelagoGA {
    public:
        // 构造函数
        ArchipelagoGA(size_t numIslands, size_t populationPerIsland)
            : m_numIslands(numIslands), m_populationPerIsland(populationPerIsland), m_migrationInterval(10), m_migrationRate(0.1), m_migrationPolicy(MigrationPolicy::BEST), m_migrationTopology(MigrationTopology::RING) {}

        // 虚析构函数
        virtual ~ArchipelagoGA() = default;

        // 初始化岛屿和初始种群
        virtual void initialize() = 0;

        // 运行算法指定的代数
        virtual void evolve(size_t generations) = 0;

        // 获取所有岛屿中的最佳解决方案
        virtual std::pair<Genotype, Phenotype> getBestSolution() const = 0;

        // 获取最佳解的适应度值
        virtual FitnessType getBestFitness() const = 0;

        // 设置迁移间隔（多少代进行一次迁移）
        virtual void setMigrationInterval(size_t interval)
        {
            m_migrationInterval = interval;
        }

        // 设置迁移率（每次迁移的个体比例）
        virtual void setMigrationRate(double rate)
        {
            m_migrationRate = rate;
        }

        // 设置迁移策略
        virtual void setMigrationPolicy(MigrationPolicy policy)
        {
            m_migrationPolicy = policy;
        }

        // 设置迁移拓扑结构
        virtual void setMigrationTopology(MigrationTopology topology)
        {
            m_migrationTopology = topology;
        }

        // 获取当前迁移间隔
        size_t getMigrationInterval() const { return m_migrationInterval; }

        // 获取当前迁移率
        double getMigrationRate() const { return m_migrationRate; }

        // 获取当前迁移策略
        MigrationPolicy getMigrationPolicy() const { return m_migrationPolicy; }

        // 获取当前迁移拓扑结构
        MigrationTopology getMigrationTopology() const { return m_migrationTopology; }

    protected:
        // 执行迁移操作
        virtual void migrateIndividuals() = 0;

        // 根据迁移策略选择迁移个体
        virtual std::vector<Genotype> selectMigrants(size_t islandIndex, size_t count) = 0;

        // 根据拓扑结构获取目标岛屿
        virtual std::vector<size_t> getDestinationIslands(size_t sourceIsland) = 0;

        // 构建迁移拓扑
        virtual void buildMigrationTopology() = 0;

        // 岛屿数量
        size_t m_numIslands;

        // 每个岛屿的种群大小
        size_t m_populationPerIsland;

        // 多少代进行一次迁移
        size_t m_migrationInterval;

        // 每个岛屿有多少比例的个体会迁移
        double m_migrationRate;

        // 迁移策略
        MigrationPolicy m_migrationPolicy;

        // 迁移拓扑结构
        MigrationTopology m_migrationTopology;

        // 迁移拓扑矩阵（记录岛屿间的连接关系）
        std::vector<std::vector<bool>> m_topologyMatrix;
};

}    // namespace algorithm
}    // namespace rtd
