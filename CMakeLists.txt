cmake_minimum_required(VERSION 4.0.0)
project(ChipTestDispatchSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(nlohmann_json REQUIRED)

add_subdirectory(client)
