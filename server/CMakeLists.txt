cmake_minimum_required(VERSION 4.0.0)
project(RTD_Server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Network Sql)

# 源文件
set(SOURCES
    src/main.cpp
    src/Server.cpp
    
    # 配置模块
    src/config/ConfigManager.cpp
    
    # 数据库模块
    src/db/DatabaseManager.cpp
    src/db/ConnectionPool.cpp
    src/db/QueryExecutor.cpp
    
    # HTTP服务模块
    src/http/HttpServer.cpp
    src/http/HttpRequest.cpp
    src/http/HttpResponse.cpp
    
    # API模块
    src/api/ApiManager.cpp
)

# 头文件
set(HEADERS
    src/Server.h
    
    # 配置模块
    src/config/ConfigManager.h
    
    # 数据库模块
    src/db/DatabaseManager.h
    src/db/ConnectionPool.h
    src/db/QueryExecutor.h
    
    # HTTP服务模块
    src/http/HttpServer.h
    src/http/HttpRequest.h
    src/http/HttpResponse.h
    
    # API模块
    src/api/ApiManager.h
)

# 创建可执行文件
add_executable(rtdserver ${SOURCES} ${HEADERS})

# 链接Qt库
target_link_libraries(rtdserver PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Sql
)

# 包含目录
target_include_directories(rtdserver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 复制配置文件到构建目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/config)
file(COPY ${CMAKE_SOURCE_DIR}/config/server.json DESTINATION ${CMAKE_BINARY_DIR}/config)
file(COPY ${CMAKE_SOURCE_DIR}/config/database.json DESTINATION ${CMAKE_BINARY_DIR}/config)
file(COPY ${CMAKE_SOURCE_DIR}/config/api.json DESTINATION ${CMAKE_BINARY_DIR}/config)

# 创建日志目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)

# 设置输出目录
set_target_properties(rtdserver PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
