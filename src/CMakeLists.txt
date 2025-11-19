# FlyKylin Source

# 添加core模块
add_subdirectory(core)

set(SOURCES
    main.cpp
    ui/viewmodels/PeerListViewModel.cpp
    ui/viewmodels/ChatViewModel.cpp
    ui/widgets/PeerListWidget.cpp
    ui/windows/MainWindow.cpp
    ui/windows/ChatWindow.cpp
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
    flykylin_protocol  # Protobuf generated code
)

# 链接Abseil DLL（Protobuf依赖）
if(VCPKG_INSTALLED_DIR)
    find_library(ABSEIL_DLL_LIB 
        NAMES abseil_dll
        PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib"
        NO_DEFAULT_PATH
        REQUIRED
    )
    target_link_libraries(FlyKylin PRIVATE ${ABSEIL_DLL_LIB})
    message(STATUS "Linked Abseil DLL: ${ABSEIL_DLL_LIB}")
endif()

# ONNX Runtime
if(ONNXRUNTIME_FOUND)
    target_include_directories(FlyKylin PRIVATE ${ONNXRUNTIME_ROOT}/include)
    target_link_libraries(FlyKylin PRIVATE onnxruntime)
endif()

# Windows DLL部署
if(WIN32)
    # 获取Qt安装路径
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    
    # 使用windeployqt自动部署Qt DLL
    add_custom_command(TARGET FlyKylin POST_BUILD
        COMMAND "${QT_BIN_DIR}/windeployqt.exe" 
            --no-translations 
            --no-system-d3d-compiler
            --no-compiler-runtime
            --no-opengl-sw
            "$<TARGET_FILE:FlyKylin>"
        COMMENT "Deploying Qt libraries for FlyKylin..."
    )
    
    # 复制vcpkg安装的所有DLL（Protobuf及其依赖）
    if(VCPKG_INSTALLED_DIR AND NOT VCPKG_TARGET_TRIPLET MATCHES "static")
        set(VCPKG_BIN_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
        if(EXISTS "${VCPKG_BIN_DIR}")
            # 复制所有vcpkg DLL，包括Protobuf、Abseil和其他依赖
            file(GLOB VCPKG_DLLS "${VCPKG_BIN_DIR}/*.dll")
            foreach(DLL_FILE ${VCPKG_DLLS})
                add_custom_command(TARGET FlyKylin POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${DLL_FILE}"
                        "$<TARGET_FILE_DIR:FlyKylin>"
                    COMMENT "Deploying ${DLL_FILE}..."
                    VERBATIM
                )
            endforeach()
        endif()
    endif()
endif()

# Installation
install(TARGETS FlyKylin
    RUNTIME DESTINATION bin
)
