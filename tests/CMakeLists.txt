# FlyKylin单元测试

# 查找GoogleTest
# 在 Linux 系统上，libgtest-dev 包可能只提供源码，需要手动编译
# 或者使用 FetchContent 下载
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    # 尝试使用 pkg-config 查找
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GTEST QUIET gtest gtest_main)
    endif()
    
    if(NOT GTEST_FOUND)
        # 使用 FetchContent 下载 GoogleTest
        # Use URL download instead of git clone to avoid git safe.directory issues in CI containers
        # Note: DOWNLOAD_EXTRACT_TIMESTAMP requires CMake 3.24+, so we omit it for compatibility
        include(FetchContent)
        set(GTEST_DOWNLOAD_URL "https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz")
        FetchContent_Declare(
            googletest
            URL "${GTEST_DOWNLOAD_URL}"
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        set(GTest_FOUND TRUE)
    endif()
endif()

if(NOT GTest_FOUND AND NOT TARGET GTest::gtest AND NOT TARGET gtest)
    message(WARNING "GoogleTest not found - tests disabled")
    return()
endif()

include(GoogleTest)

# 确定 GTest 链接目标名称
# FetchContent 创建的是 gtest/gtest_main，而 find_package 创建的是 GTest::gtest/GTest::gtest_main
if(TARGET GTest::gtest)
    set(GTEST_LIBS GTest::gtest GTest::gtest_main)
else()
    set(GTEST_LIBS gtest gtest_main)
endif()

# 测试源文件
set(TEST_SOURCES
    core/config/UserProfile_test.cpp
    core/ProtobufSerializer_test.cpp  # Temporarily disabled: depends on protobuf
    # core/PeerNode_test.cpp  # TODO: 待实现
    core/PeerDiscovery_test.cpp  # TODO: 待实现
    core/services/FileTransferService_test.cpp
)

# 创建测试可执行文件
add_executable(flykylin_tests ${TEST_SOURCES})

target_link_libraries(flykylin_tests PRIVATE
    ${GTEST_LIBS}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
    flykylin_core
    flykylin_protocol
)

target_include_directories(flykylin_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/models
)

# 独立的 FileTransferService 测试可执行，便于单独运行文件传输相关用例
add_executable(flykylin_filetransfer_tests
    core/services/FileTransferService_test.cpp
)

target_link_libraries(flykylin_filetransfer_tests PRIVATE
    ${GTEST_LIBS}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
    flykylin_core
    flykylin_protocol
)

target_include_directories(flykylin_filetransfer_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/models
)

# Windows DLL部署（测试程序）
if(WIN32)
    # 获取Qt安装路径
    get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    
    # 使用windeployqt自动部署Qt DLL（主测试可执行）
    add_custom_command(TARGET flykylin_tests POST_BUILD
        COMMAND "${QT_BIN_DIR}/windeployqt.exe" 
            --no-translations 
            --no-system-d3d-compiler
            --no-compiler-runtime
            --no-opengl-sw
            "$<TARGET_FILE:flykylin_tests>"
        COMMENT "Deploying Qt libraries for tests..."
    )

    # 为独立的文件传输测试可执行部署Qt DLL
    add_custom_command(TARGET flykylin_filetransfer_tests POST_BUILD
        COMMAND "${QT_BIN_DIR}/windeployqt.exe" 
            --no-translations 
            --no-system-d3d-compiler
            --no-compiler-runtime
            --no-opengl-sw
            "$<TARGET_FILE:flykylin_filetransfer_tests>"
        COMMENT "Deploying Qt libraries for filetransfer tests..."
    )
    
    # 复制 Protobuf 运行时 DLL（不依赖 vcpkg 变量，直接使用 imported target）
    if(TARGET protobuf::libprotobuf)
        # 为 flykylin_tests 复制 libprotobuf.dll
        add_custom_command(TARGET flykylin_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:protobuf::libprotobuf>
                $<TARGET_FILE_DIR:flykylin_tests>
            COMMENT "Copying protobuf runtime for flykylin_tests..."
        )

        # 为 flykylin_filetransfer_tests 复制 libprotobuf.dll
        add_custom_command(TARGET flykylin_filetransfer_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:protobuf::libprotobuf>
                $<TARGET_FILE_DIR:flykylin_filetransfer_tests>
            COMMENT "Copying protobuf runtime for flykylin_filetransfer_tests..."
        )
    endif()
endif()

# 注册测试（禁用自动发现以避免POST_BUILD阶段DLL依赖问题）
# gtest_discover_tests(flykylin_tests)
add_test(NAME flykylin_tests COMMAND flykylin_tests)
