set(CLIENT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/models/GenericTableModel.cpp
    src/models/LotModel.cpp
    src/models/EquipmentModel.cpp
    src/models/DispatchModel.cpp
    src/views/DispatchView.cpp
    src/views/LotListView.cpp
    src/views/EquipmentListView.cpp
    src/ApiClient.cpp
    src/utils/StyleHelper.cpp
    resources/resources.qrc
)

set(CLIENT_HEADERS
    src/MainWindow.h
    src/models/GenericTableModel.h
    src/models/LotModel.h
    src/models/EquipmentModel.h
    src/models/DispatchModel.h
    src/views/DispatchView.h
    src/views/LotListView.h
    src/views/EquipmentListView.h
    src/ApiClient.h
    src/utils/StyleHelper.h
)

add_executable(client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(client PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    nlohmann_json::nlohmann_json
)

# 设置输出路径
set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 复制配置文件到输出目录
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_BINARY_DIR}/bin/config.json" COPYONLY)
