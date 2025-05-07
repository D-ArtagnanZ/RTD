#include "job_scheduler_impl.h"
#include <chrono>
#include <thread>

namespace rtd {
namespace schedule {

// 实现多岛遗传算法的派工调度器
class JobSchedulerImpl::SchedulerGA: public algorithm::ArchipelagoGA<Chromosome, Schedule, double> {
    public:
        SchedulerGA(
          size_t                                  numIslands,
          size_t                                  populationPerIsland,
          size_t                                  lotCount,
          size_t                                  machineCount,
          const std::vector<std::vector<double>> &processingTimes,
          const std::vector<std::string>         &lotIds,
          const std::vector<std::string>         &machineIds,
          double                                  crossoverRate,
          double                                  mutationRate,
          size_t                                  elitismCount)
            : algorithm::ArchipelagoGA<Chromosome, Schedule, double>(numIslands, populationPerIsland), m_lotCount(lotCount), m_machineCount(machineCount), m_processingTimes(processingTimes), m_lotIds(lotIds), m_machineIds(machineIds), m_crossoverRate(crossoverRate), m_mutationRate(mutationRate), m_elitismCount(elitismCount), m_evaluator(lotCount, machineCount, processingTimes), m_bestFitness(-std::numeric_limits<double>::max())
        {
            // 使用时间种子初始化随机数生成器
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            m_rng.seed(seed);
        }

        void initialize() override
        {
            m_populations.resize(m_numIslands);
            m_fitness.resize(m_numIslands);

            // 初始化每个岛的种群
            for (size_t island = 0; island < m_numIslands; ++island) {
                m_populations[island].resize(m_populationPerIsland);
                m_fitness[island].resize(m_populationPerIsland);

                // 创建初始随机染色体
                for (size_t i = 0; i < m_populationPerIsland; ++i) {
                    m_populations[island][i] = Chromosome::createRandom(
                      m_lotCount, m_machineCount, m_processingTimes, m_rng);

                    // 评估适应度
                    m_fitness[island][i] = m_evaluator.evaluate(m_populations[island][i]);

                    // 更新最佳解
                    if (m_fitness[island][i] > m_bestFitness) {
                        m_bestFitness    = m_fitness[island][i];
                        m_bestChromosome = m_populations[island][i];

                        // 更新最佳解的表现型
                        m_bestPhenotype.clear();
                        m_evaluator.evaluateAndUpdate(
                          m_bestChromosome, m_bestPhenotype, m_lotIds, m_machineIds);
                    }
                }
            }

            // 构建迁移拓扑
            buildMigrationTopology();
        }

        void evolve(size_t generations) override
        {
            for (size_t gen = 0; gen < generations; ++gen) {
                // 每个岛独立演化
                std::vector<std::thread> threads;
                for (size_t island = 0; island < m_numIslands; ++island) {
                    threads.push_back(std::thread(&SchedulerGA::evolveIsland, this, island));
                }

                // 等待所有岛演化完成
                for (auto &t: threads) {
                    if (t.joinable()) {
                        t.join();
                    }
                }

                // 周期性迁移个体
                if ((gen + 1) % m_migrationInterval == 0) {
                    migrateIndividuals();
                }
            }
        }

        std::pair<Chromosome, Schedule> getBestSolution() const override
        {
            return {m_bestChromosome, m_bestPhenotype};
        }

        double getBestFitness() const override
        {
            return m_bestFitness;
        }

    protected:
        void migrateIndividuals() override
        {
            // 根据拓扑和迁移策略执行迁移
            for (size_t sourceIsland = 0; sourceIsland < m_numIslands; ++sourceIsland) {
                // 获取目标岛
                std::vector<size_t> destinations = getDestinationIslands(sourceIsland);

                // 计算每个目标岛要接收的移民数量
                size_t migrantCount = static_cast<size_t>(m_populationPerIsland * m_migrationRate);
                if (migrantCount == 0) migrantCount = 1;    // 至少迁移一个个体

                // 选择迁移个体
                std::vector<Chromosome> migrants = selectMigrants(sourceIsland, migrantCount);

                // 发送移民到每个目标岛
                for (size_t destIsland: destinations) {
                    // 在目标岛中替换最差的个体
                    for (size_t i = 0; i < migrants.size(); ++i) {
                        // 找出目标岛中最差的个体
                        size_t worstIdx     = 0;
                        double worstFitness = m_fitness[destIsland][0];

                        for (size_t j = 1; j < m_populationPerIsland; ++j) {
                            if (m_fitness[destIsland][j] < worstFitness) {
                                worstFitness = m_fitness[destIsland][j];
                                worstIdx     = j;
                            }
                        }

                        // 如果移民更好，则替换
                        double migrantFitness = m_evaluator.evaluate(migrants[i]);
                        if (migrantFitness > worstFitness) {
                            m_populations[destIsland][worstIdx] = migrants[i];
                            m_fitness[destIsland][worstIdx]     = migrantFitness;

                            // 更新全局最佳解
                            if (migrantFitness > m_bestFitness) {
                                m_bestFitness    = migrantFitness;
                                m_bestChromosome = migrants[i];

                                // 更新最佳解的表现型
                                m_bestPhenotype.clear();
                                m_evaluator.evaluateAndUpdate(
                                  m_bestChromosome, m_bestPhenotype, m_lotIds, m_machineIds);
                            }
                        }
                    }
                }
            }
        }

        std::vector<Chromosome> selectMigrants(size_t islandIndex, size_t count) override
        {
            // 根据迁移策略选择移民，这里使用精英选择
            std::vector<std::pair<double, size_t>> fitnessIndices;
            for (size_t i = 0; i < m_populationPerIsland; ++i) {
                fitnessIndices.push_back({m_fitness[islandIndex][i], i});
            }

            // 按适应度排序
            std::sort(fitnessIndices.begin(), fitnessIndices.end(), [](const auto &a, const auto &b) { return a.first > b.first; });

            // 选择前count个作为移民
            std::vector<Chromosome> migrants;
            for (size_t i = 0; i < count && i < fitnessIndices.size(); ++i) {
                migrants.push_back(m_populations[islandIndex][fitnessIndices[i].second]);
            }

            return migrants;
        }

        std::vector<size_t> getDestinationIslands(size_t sourceIsland) override
        {
            std::vector<size_t> destinations;

            for (size_t i = 0; i < m_numIslands; ++i) {
                if (i != sourceIsland && m_topologyMatrix[sourceIsland][i]) {
                    destinations.push_back(i);
                }
            }

            return destinations;
        }

        void buildMigrationTopology() override
        {
            // 构建迁移拓扑矩阵
            m_topologyMatrix.resize(m_numIslands, std::vector<bool>(m_numIslands, false));

            // 根据迁移拓扑设置连接关系
            switch (m_migrationTopology) {
                case algorithm::MigrationTopology::RING:
                    // 环形拓扑：每个岛只与相邻岛连接
                    for (size_t i = 0; i < m_numIslands; ++i) {
                        m_topologyMatrix[i][(i + 1) % m_numIslands]                = true;
                        m_topologyMatrix[i][(i + m_numIslands - 1) % m_numIslands] = true;
                    }
                    break;

                case algorithm::MigrationTopology::FULLY_CONNECTED:
                    // 全连接拓扑：每个岛与所有其他岛连接
                    for (size_t i = 0; i < m_numIslands; ++i) {
                        for (size_t j = 0; j < m_numIslands; ++j) {
                            if (i != j) {
                                m_topologyMatrix[i][j] = true;
                            }
                        }
                    }
                    break;

                case algorithm::MigrationTopology::STAR:
                    // 星形拓扑：中心岛(0)与所有其他岛连接
                    for (size_t i = 1; i < m_numIslands; ++i) {
                        m_topologyMatrix[0][i] = true;
                        m_topologyMatrix[i][0] = true;
                    }
                    break;

                case algorithm::MigrationTopology::MESH:
                    // 网格拓扑：每个岛与一定范围内的岛连接
                    size_t meshSize = static_cast<size_t>(std::sqrt(m_numIslands));
                    for (size_t i = 0; i < m_numIslands; ++i) {
                        size_t row = i / meshSize;
                        size_t col = i % meshSize;

                        // 连接上下左右的邻居
                        if (row > 0) m_topologyMatrix[i][i - meshSize] = true;
                        if (row < meshSize - 1) m_topologyMatrix[i][i + meshSize] = true;
                        if (col > 0) m_topologyMatrix[i][i - 1] = true;
                        if (col < meshSize - 1) m_topologyMatrix[i][i + 1] = true;
                    }
                    break;
            }
        }

    private:
        // 参数
        size_t                           m_lotCount;
        size_t                           m_machineCount;
        std::vector<std::vector<double>> m_processingTimes;
        std::vector<std::string>         m_lotIds;
        std::vector<std::string>         m_machineIds;
        double                           m_crossoverRate;
        double                           m_mutationRate;
        size_t                           m_elitismCount;

        // 随机数生成
        std::mt19937 m_rng;

        // 种群和适应度
        std::vector<std::vector<Chromosome>> m_populations;
        std::vector<std::vector<double>>     m_fitness;

        // 最优解
        Chromosome m_bestChromosome;
        Schedule   m_bestPhenotype;
        double     m_bestFitness;

        // 评估器
        ScheduleEvaluator m_evaluator;

        /**
         * 演化单个岛
         */
        void evolveIsland(size_t island)
        {
            // 创建新一代种群
            std::vector<Chromosome> newPopulation;
            std::vector<double>     newFitness;

            // 精英保留
            std::vector<std::pair<double, size_t>> sortedIndices;
            for (size_t i = 0; i < m_populationPerIsland; ++i) {
                sortedIndices.push_back({m_fitness[island][i], i});
            }
            std::sort(sortedIndices.begin(), sortedIndices.end(), [](const auto &a, const auto &b) { return a.first > b.first; });

            // 复制精英到新种群
            for (size_t i = 0; i < m_elitismCount && i < sortedIndices.size(); ++i) {
                size_t idx = sortedIndices[i].second;
                newPopulation.push_back(m_populations[island][idx]);
                newFitness.push_back(m_fitness[island][idx]);
            }

            // 通过选择、交叉和变异生成剩余个体
            while (newPopulation.size() < m_populationPerIsland) {
                // 选择两个父代
                size_t parent1Idx = tournamentSelect(island);
                size_t parent2Idx = tournamentSelect(island);

                // 交叉
                Chromosome child1 = m_populations[island][parent1Idx];
                Chromosome child2 = m_populations[island][parent2Idx];

                if (std::uniform_real_distribution<double>(0.0, 1.0)(m_rng) < m_crossoverRate) {
                    child1 = child1.crossover(m_populations[island][parent2Idx], m_rng);
                    child2 = child2.crossover(m_populations[island][parent1Idx], m_rng);
                }

                // 变异
                child1.mutate(m_mutationRate, m_rng);
                child2.mutate(m_mutationRate, m_rng);

                // 修复无效染色体
                child1.repair(m_lotCount, m_machineCount, m_processingTimes, m_rng);
                child2.repair(m_lotCount, m_machineCount, m_processingTimes, m_rng);

                // 评估新个体
                double fitness1 = m_evaluator.evaluate(child1);
                double fitness2 = m_evaluator.evaluate(child2);

                // 添加到新种群
                newPopulation.push_back(child1);
                newFitness.push_back(fitness1);

                if (newPopulation.size() < m_populationPerIsland) {
                    newPopulation.push_back(child2);
                    newFitness.push_back(fitness2);
                }

                // 更新全局最佳解
                if (fitness1 > m_bestFitness) {
                    m_bestFitness    = fitness1;
                    m_bestChromosome = child1;

                    // 更新最佳解的表现型
                    m_bestPhenotype.clear();
                    m_evaluator.evaluateAndUpdate(
                      m_bestChromosome, m_bestPhenotype, m_lotIds, m_machineIds);
                }

                if (fitness2 > m_bestFitness) {
                    m_bestFitness    = fitness2;
                    m_bestChromosome = child2;

                    // 更新最佳解的表现型
                    m_bestPhenotype.clear();
                    m_evaluator.evaluateAndUpdate(
                      m_bestChromosome, m_bestPhenotype, m_lotIds, m_machineIds);
                }
            }

            // 更新种群
            m_populations[island] = std::move(newPopulation);
            m_fitness[island]     = std::move(newFitness);
        }

        /**
         * 锦标赛选择
         */
        size_t tournamentSelect(size_t island)
        {
            static const size_t TOURNAMENT_SIZE = 3;

            std::uniform_int_distribution<size_t> dist(0, m_populationPerIsland - 1);

            size_t bestIdx     = dist(m_rng);
            double bestFitness = m_fitness[island][bestIdx];

            for (size_t i = 1; i < TOURNAMENT_SIZE; ++i) {
                size_t idx = dist(m_rng);
                if (m_fitness[island][idx] > bestFitness) {
                    bestIdx     = idx;
                    bestFitness = m_fitness[island][idx];
                }
            }

            return bestIdx;
        }
};

JobSchedulerImpl::JobSchedulerImpl()
    : m_populationSize(100), m_generationCount(200), m_islandCount(4), m_crossoverRate(0.8), m_mutationRate(0.2), m_elitismCount(2), m_migrationInterval(10), m_migrationRate(0.1)
{
    // 使用时间种子初始化随机数生成器
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    m_rng.seed(seed);
}

void JobSchedulerImpl::setLots(const std::vector<std::string> &lotIds)
{
    m_lotIds = lotIds;
}

void JobSchedulerImpl::setMachines(const std::vector<std::string> &machineIds)
{
    m_machineIds = machineIds;
}

bool JobSchedulerImpl::setProcessingTimes(const std::vector<std::vector<double>> &processingTimes)
{
    if (m_lotIds.empty() || m_machineIds.empty()) {
        return false;
    }

    // 检查处理时间矩阵的维度
    if (processingTimes.size() != m_lotIds.size()) {
        return false;
    }

    for (const auto &row: processingTimes) {
        if (row.size() != m_machineIds.size()) {
            return false;
        }
    }

    m_processingTimes = processingTimes;
    return true;
}

bool JobSchedulerImpl::setProcessingTime(size_t lotIndex, size_t machineIndex, double time)
{
    if (lotIndex >= m_lotIds.size() || machineIndex >= m_machineIds.size()) {
        return false;
    }

    // 如果处理时间矩阵未初始化，则初始化
    if (m_processingTimes.empty()) {
        m_processingTimes.resize(m_lotIds.size(),
                                 std::vector<double>(m_machineIds.size(), 0.0));
    }

    m_processingTimes[lotIndex][machineIndex] = time;
    return true;
}

Schedule JobSchedulerImpl::calculateSchedule()
{
    validateInputs();

    if (!isValidProblem()) {
        return Schedule();
    }

    // 创建并配置遗传算法
    SchedulerGA ga(
      m_islandCount,
      m_populationSize / m_islandCount,
      m_lotIds.size(),
      m_machineIds.size(),
      m_processingTimes,
      m_lotIds,
      m_machineIds,
      m_crossoverRate,
      m_mutationRate,
      m_elitismCount);

    // 设置迁移参数
    ga.setMigrationInterval(m_migrationInterval);
    ga.setMigrationRate(m_migrationRate);

    // 初始化并运行算法
    ga.initialize();
    ga.evolve(m_generationCount);

    // 返回最佳解
    auto solution = ga.getBestSolution();
    return solution.second;
}

std::future<Schedule> JobSchedulerImpl::calculateScheduleAsync()
{
    return std::async(std::launch::async, [this]() {
        return this->calculateSchedule();
    });
}

bool JobSchedulerImpl::isValidProblem() const
{
    if (m_lotIds.empty() || m_machineIds.empty() || m_processingTimes.empty()) {
        return false;
    }

    // 检查是否至少有一个可行的分配
    for (size_t i = 0; i < m_lotIds.size(); ++i) {
        bool hasValidMachine = false;
        for (size_t j = 0; j < m_machineIds.size(); ++j) {
            if (m_processingTimes[i][j] > 0) {
                hasValidMachine = true;
                break;
            }
        }

        if (!hasValidMachine) {
            return false;    // 至少有一个批次没有可用的机台
        }
    }

    return true;
}

void JobSchedulerImpl::validateInputs()
{
    // 检查并初始化处理时间矩阵
    if (m_processingTimes.empty() && !m_lotIds.empty() && !m_machineIds.empty()) {
        m_processingTimes.resize(m_lotIds.size(),
                                 std::vector<double>(m_machineIds.size(), 0.0));
    }
}

Schedule JobSchedulerImpl::decodeChromosome(const Chromosome &chromosome)
{
    Schedule schedule;

    // 创建评估器
    ScheduleEvaluator evaluator(
      m_lotIds.size(),
      m_machineIds.size(),
      m_processingTimes);

    // 评估并更新派工方案
    evaluator.evaluateAndUpdate(chromosome, schedule, m_lotIds, m_machineIds);
    return schedule;
}

std::unique_ptr<JobScheduler> JobScheduler::create()
{
    return std::make_unique<JobSchedulerImpl>();
}

}    // namespace schedule
}    // namespace rtd
