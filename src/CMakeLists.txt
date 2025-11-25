# FlyKylin Source

# 添加core模块
add_subdirectory(core)

set(SOURCES
    main.cpp
    ui/viewmodels/PeerListViewModel.cpp
    ui/viewmodels/ChatViewModel.cpp
    ui/viewmodels/SettingsViewModel.cpp
    ui/widgets/PeerListWidget.cpp
    ui/windows/MainWindow.cpp
    ui/windows/ChatWindow.cpp
)

add_executable(FlyKylin ${SOURCES})

if(WIN32)
    # 使用 src/ui/resources 下的应用图标资源
    set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ui/resources/app_icon.rc")
    if(EXISTS "${APP_ICON_RESOURCE}")
        target_sources(FlyKylin PRIVATE ${APP_ICON_RESOURCE})
    endif()
endif()

target_link_libraries(FlyKylin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Quick
    flykylin_core
    flykylin_protocol
)

# QCoro (Coroutines) - Use FetchContent to use system Qt
include(FetchContent)
FetchContent_Declare(
    qcoro
    GIT_REPOSITORY https://github.com/qcoro/qcoro.git
    GIT_TAG        v0.9.0
    OVERRIDE_FIND_PACKAGE
)
# Disable QCoro tests and examples to speed up build
set(QCORO_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(QCORO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# Disable QCoro WebSockets to avoid dependency on Qt6WebSockets
set(QCORO_WITH_QTWEBSOCKETS OFF CACHE BOOL "" FORCE)
# We only use QCoro Core/Network; disable QML/Quick support to avoid
# depending on Qt private Qml/Quick headers, which are not available
# in the Ubuntu 24.04 ARM64 system Qt packages.
set(QCORO_WITH_QML OFF CACHE BOOL "" FORCE)
set(QCORO_WITH_QTQUICK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(qcoro)

target_link_libraries(FlyKylin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Quick
    flykylin_core
    QCoro6::Core
    QCoro6::Network
)

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
    
    # 使用windeployqt自动部署Qt DLL 和 QML 模块
    add_custom_command(TARGET FlyKylin POST_BUILD
        COMMAND "${QT_BIN_DIR}/windeployqt.exe"
            --no-translations
            --no-system-d3d-compiler
            --no-compiler-runtime
            --no-opengl-sw
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/ui/qml"
            "$<TARGET_FILE:FlyKylin>"
        COMMENT "Deploying Qt libraries and QML modules for FlyKylin..."
    )

    # 复制 Protobuf 运行时 DLL（不依赖 vcpkg 变量，直接使用 imported target）
    if(TARGET protobuf::libprotobuf)
        add_custom_command(TARGET FlyKylin POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:protobuf::libprotobuf>
                $<TARGET_FILE_DIR:FlyKylin>
            COMMENT "Copying protobuf runtime for FlyKylin..."
        )
    endif()

    # 如果 Zlib/Abseil 等来自统一前缀（例如 Anaconda/Library），则一并复制其运行时 DLL
    if(ZLIB_INCLUDE_DIR)
        # ZLIB_INCLUDE_DIR 通常为 "<prefix>/include"，推导出 "<prefix>/bin"
        get_filename_component(THIRDPARTY_ROOT "${ZLIB_INCLUDE_DIR}" DIRECTORY)
        set(THIRDPARTY_BIN_DIR "${THIRDPARTY_ROOT}/bin")

        if(EXISTS "${THIRDPARTY_BIN_DIR}")
            # abseil_dll.dll（Abseil 运行时，protobuf 依赖）
            if(EXISTS "${THIRDPARTY_BIN_DIR}/abseil_dll.dll")
                add_custom_command(TARGET FlyKylin POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${THIRDPARTY_BIN_DIR}/abseil_dll.dll"
                        "$<TARGET_FILE_DIR:FlyKylin>"
                    COMMENT "Copying abseil_dll.dll runtime for FlyKylin..."
                )
            endif()

            # zlib.dll 或 zlib1.dll（部分依赖可能需要）
            if(EXISTS "${THIRDPARTY_BIN_DIR}/zlib.dll")
                add_custom_command(TARGET FlyKylin POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${THIRDPARTY_BIN_DIR}/zlib.dll"
                        "$<TARGET_FILE_DIR:FlyKylin>"
                    COMMENT "Copying zlib.dll runtime for FlyKylin..."
                )
            elseif(EXISTS "${THIRDPARTY_BIN_DIR}/zlib1.dll")
                add_custom_command(TARGET FlyKylin POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${THIRDPARTY_BIN_DIR}/zlib1.dll"
                        "$<TARGET_FILE_DIR:FlyKylin>"
                    COMMENT "Copying zlib1.dll runtime for FlyKylin..."
                )
            endif()
        endif()
    endif()
    
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

# Copy QML files to build directory
add_custom_command(TARGET FlyKylin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/qml"
        "$<TARGET_FILE_DIR:FlyKylin>/src/ui/qml"
    COMMENT "Copying QML files..."
)

# Copy assets (fonts, images) to build directory
add_custom_command(TARGET FlyKylin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/assets"
        "$<TARGET_FILE_DIR:FlyKylin>/src/ui/assets"
    COMMENT "Copying assets..."
)

# Copy resources (icons, app logo, etc.) to build directory
add_custom_command(TARGET FlyKylin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/resources"
        "$<TARGET_FILE_DIR:FlyKylin>/src/ui/resources"
    COMMENT "Copying resources..."
)

# Installation
install(TARGETS FlyKylin
    RUNTIME DESTINATION bin
)
