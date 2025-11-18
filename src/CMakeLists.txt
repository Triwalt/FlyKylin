# FlyKylin Source

# 添加core模块
add_subdirectory(core)

set(SOURCES
    main.cpp
    ui/viewmodels/PeerListViewModel.cpp
    ui/widgets/PeerListWidget.cpp
    ui/windows/MainWindow.cpp
)

add_executable(FlyKylin ${SOURCES})

target_link_libraries(FlyKylin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Concurrent
    flykylin_core
)

# ONNX Runtime
if(ONNXRUNTIME_FOUND)
    target_include_directories(FlyKylin PRIVATE ${ONNXRUNTIME_ROOT}/include)
    target_link_libraries(FlyKylin PRIVATE onnxruntime)
endif()

# Installation
install(TARGETS FlyKylin
    RUNTIME DESTINATION bin
)
