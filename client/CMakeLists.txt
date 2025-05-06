cmake_minimum_required(VERSION 3.16)
project(RTDClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理moc、uic、rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 确保找得到UI文件
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Charts)

# 添加资源文件目录
add_subdirectory(resources)

# 暂时禁用资源编译以解决图像缺失问题
set(RESOURCES
    resources/resources.qrc
)

# 源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/apiclient.cpp
    src/equipmentwidget.cpp
    src/lotwidget.cpp
    src/dispatchwidget.cpp
    src/models.cpp
    src/createtaskdialog.cpp
)

# 头文件
set(HEADERS
    src/mainwindow.h
    src/apiclient.h
    src/equipmentwidget.h
    src/lotwidget.h
    src/dispatchwidget.h
    src/models.h
    src/createtaskdialog.h
)

# UI文件
set(FORMS
    src/ui/mainwindow.ui
    src/ui/equipmentwidget.ui
    src/ui/lotwidget.ui
    src/ui/dispatchwidget.ui
    src/ui/createtaskdialog.ui
)

# 确保UI文件目录存在
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)

# 创建可执行文件
add_executable(rtdclient
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

# 确保包含目录
target_include_directories(rtdclient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# 链接Qt库
target_link_libraries(rtdclient PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
)

# 安装目标
install(TARGETS rtdclient
    RUNTIME DESTINATION bin
)

# 安装配置文件和资源
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources
    DESTINATION share/rtdclient
)
