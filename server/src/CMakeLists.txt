cmake_minimum_required(VERSION 3.20)
project(RTDPlus VERSION 1.0.0 LANGUAGES CXX)

# 设置C++20标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(SOCI REQUIRED)
find_package(ODBC REQUIRED)
find_package(nlohmann_json 3.9.0 REQUIRED)
find_package(Crow REQUIRED)
find_package(OpenSSL REQUIRED)  # 添加OpenSSL
find_package(ZLIB REQUIRED)     # 添加zlib

# 包含目录
include_directories(
    ${SOCI_INCLUDE_DIRS}
    ${ODBC_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}     # OpenSSL头文件目录
    ${ZLIB_INCLUDE_DIRS}       # zlib头文件目录
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 添加可执行文件
add_executable(rtd_server
    src/main.cpp
    src/db_manager.cpp
    src/api_handler.cpp
    src/http_server.cpp
)

# 链接库
target_link_libraries(rtd_server
    PRIVATE
    ${SOCI_LIBRARIES}
    ${ODBC_LIBRARIES}
    SOCI::Core
    SOCI::ODBC
    nlohmann_json::nlohmann_json
    Crow::Crow
    ${OPENSSL_LIBRARIES}     # 添加OpenSSL库
    ZLIB::ZLIB              # 添加zlib库
    ssl                     # 明确链接SSL库
    crypto                  # 明确链接crypto库
)

# 安装目标
install(TARGETS rtd_server
    RUNTIME DESTINATION bin
)

# 复制配置文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config
    DESTINATION .
)
