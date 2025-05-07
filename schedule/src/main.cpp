#include "job_scheduler.h"
#include "schedule_data_manager.h"
#include <atomic>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

using namespace rtd::schedule;

// 全局变量用于处理信号
std::atomic<bool> g_running{true};

// 信号处理函数
void signalHandler(int signal)
{
    std::cout << "接收到信号：" << signal << "，准备退出..." << std::endl;
    g_running = false;
}

// 获取当前时间戳
std::string getCurrentTimestamp()
{
    auto              now  = std::chrono::system_clock::now();
    auto              time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int main(int argc, char *argv[])
{
    try {
        // 设置信号处理
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        std::cout << "RTD+ 调度引擎启动..." << std::endl;

        // 从命令行参数解析配置
        int scheduleIntervalSeconds = 300;    // 默认5分钟重新计算一次

        if (argc > 1) {
            scheduleIntervalSeconds = std::stoi(argv[1]);
        }

        std::cout << "调度计算周期: " << scheduleIntervalSeconds << " 秒" << std::endl;

        // 初始化数据管理器
        auto dataManager = ScheduleDataManager::create();
        if (!dataManager->initialize()) {
            std::cerr << "数据管理器初始化失败" << std::endl;
            return 1;
        }

        std::cout << "数据管理器初始化成功" << std::endl;

        // 主调度循环
        while (g_running) {
            std::cout << "\n======== " << getCurrentTimestamp() << " 开始新一轮调度计算 ========" << std::endl;

            try {
                // 获取所有设备和批次
                std::vector<std::string> equipments = dataManager->getAllEquipments();
                std::vector<std::string> lots       = dataManager->getAllLots();

                std::cout << "发现 " << equipments.size() << " 台设备和 " << lots.size() << " 个批次" << std::endl;

                if (equipments.empty() || lots.empty()) {
                    std::cout << "没有找到设备或批次，等待下一轮调度" << std::endl;
                }
                else {
                    // 获取处理时间矩阵
                    std::vector<std::vector<double>> processingTimes =
                      dataManager->getProcessTimeMatrix(lots, equipments);

                    std::cout << "处理时间矩阵加载完成" << std::endl;

                    // 输出工艺兼容性信息
                    int compatiblePairs = 0;
                    for (size_t i = 0; i < lots.size(); ++i) {
                        for (size_t j = 0; j < equipments.size(); ++j) {
                            if (processingTimes[i][j] > 0) {
                                compatiblePairs++;
                            }
                        }
                    }
                    std::cout << "工艺兼容性：" << compatiblePairs << " 个有效配对（非零处理时间）" << std::endl;

                    // 创建调度器
                    auto scheduler = JobScheduler::create();
                    scheduler->setLots(lots);
                    scheduler->setMachines(equipments);
                    scheduler->setProcessingTimes(processingTimes);

                    // 设置调度参数
                    scheduler->setPopulationSize(100);
                    scheduler->setGenerationCount(200);
                    scheduler->setIslandCount(4);
                    scheduler->setCrossoverRate(0.8);
                    scheduler->setMutationRate(0.2);
                    scheduler->setElitismCount(2);
                    scheduler->setMigrationInterval(10);
                    scheduler->setMigrationRate(0.1);

                    std::cout << "开始计算调度方案..." << std::endl;
                    auto startTime = std::chrono::high_resolution_clock::now();

                    // 计算调度方案
                    Schedule schedule = scheduler->calculateSchedule();

                    auto                          endTime = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = endTime - startTime;

                    std::cout << "调度计算完成，耗时 " << elapsed.count() << " 秒" << std::endl;
                    std::cout << "完工时间: " << schedule.makespan << std::endl;
                    std::cout << "平均流通时间: " << schedule.meanFlowTime << std::endl;

                    // 保存调度结果到数据库
                    std::cout << "保存调度结果到数据库..." << std::endl;

                    // 如果找到了有效的调度方案
                    if (!schedule.assignments.empty()) {
                        std::vector<std::tuple<std::string, std::string, double, double, double>> results;
                        double                                                                    releaseTime = static_cast<double>(std::chrono::system_clock::to_time_t(
                          std::chrono::system_clock::now()));

                        // 对每个机台处理分配的批次
                        for (size_t i = 0; i < schedule.machineAssignments.size(); ++i) {
                            const auto &machineJobs = schedule.machineAssignments[i];
                            std::string equipmentId = i < equipments.size() ? equipments[i] : "Unknown";

                            for (const auto &job: machineJobs) {
                                // 确保处理时间大于0（有效配置）
                                if (job.processingTime > 0) {
                                    results.push_back(std::make_tuple(
                                      equipmentId,
                                      job.lotId,
                                      releaseTime,
                                      job.startTime,
                                      job.endTime));
                                }
                            }
                        }

                        if (!results.empty()) {
                            if (dataManager->saveDispatchResults(results)) {
                                std::cout << "成功保存 " << results.size() << " 条调度记录" << std::endl;
                            }
                            else {
                                std::cerr << "保存调度结果时出错" << std::endl;
                            }
                        }
                        else {
                            std::cout << "没有找到有效的调度方案" << std::endl;
                        }
                    }
                    else {
                        std::cout << "没有找到有效的调度方案" << std::endl;
                    }
                }
            }
            catch (const std::exception &e) {
                std::cerr << "调度计算过程中发生错误: " << e.what() << std::endl;
                std::cerr << "将在下一轮重试" << std::endl;
            }

            std::cout << "======== 本轮调度计算结束 ========" << std::endl;

            // 等待下一次调度周期，同时定期检查是否收到退出信号
            for (int i = 0; i < scheduleIntervalSeconds && g_running; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        std::cout << "RTD+ 调度引擎正常退出" << std::endl;
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
}
