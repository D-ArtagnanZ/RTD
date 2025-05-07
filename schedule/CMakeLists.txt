cmake_minimum_required(VERSION 4.0.0)
project(RTDSchedule VERSION 1.0.0 LANGUAGES CXX)

# 设置C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(SOCI REQUIRED)
find_package(ODBC REQUIRED)
find_package(nlohmann_json 3.9.0 REQUIRED)
find_package(Threads REQUIRED)

# 包含目录
include_directories(
    ${SOCI_INCLUDE_DIRS}
    ${ODBC_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 添加源文件
set(SOURCES
    src/main.cpp
    src/schedule_data_manager.cpp
    src/job_scheduler_impl.cpp
    src/schedule_chromosome.cpp
    src/schedule_evaluator.cpp
)

# 添加可执行文件
add_executable(rtd_schedule ${SOURCES})

# 链接库
target_link_libraries(rtd_schedule
    PRIVATE
    ${SOCI_LIBRARIES}
    ${ODBC_LIBRARIES}
    SOCI::Core
    SOCI::ODBC
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# 安装目标
install(TARGETS rtd_schedule
    RUNTIME DESTINATION bin
)

# 复制配置文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config
    DESTINATION .
)

# 设置DSN目录
add_custom_command(
    TARGET rtd_schedule POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/config/dsn
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config/dsn/oracle.dsn
        ${CMAKE_CURRENT_SOURCE_DIR}/config/dsn/postgresql.dsn
        ${CMAKE_BINARY_DIR}/config/dsn/
)
