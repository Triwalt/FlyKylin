# FlyKylin单元测试

# 查找GoogleTest
find_package(GTest)

if(NOT GTest_FOUND)
    message(WARNING "GoogleTest not found - tests disabled")
    return()
endif()

include(GoogleTest)

# 测试源文件
set(TEST_SOURCES
    core/config/UserProfile_test.cpp
    core/ProtobufSerializer_test.cpp
    # core/PeerNode_test.cpp  # TODO: 待实现
    # core/PeerDiscovery_test.cpp  # TODO: 待实现
)

# 创建测试可执行文件
add_executable(flykylin_tests ${TEST_SOURCES})

target_link_libraries(flykylin_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    Qt6::Core
    Qt6::Network
    Qt6::Test
    flykylin_core
    flykylin_protocol
)

target_include_directories(flykylin_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/core/models
)

# Windows DLL部署（测试程序）
if(WIN32)
    # 获取Qt安装路径
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    
    # 使用windeployqt自动部署Qt DLL
    add_custom_command(TARGET flykylin_tests POST_BUILD
        COMMAND "${QT_BIN_DIR}/windeployqt.exe" 
            --no-translations 
            --no-system-d3d-compiler
            --no-compiler-runtime
            --no-opengl-sw
            "$<TARGET_FILE:flykylin_tests>"
        COMMENT "Deploying Qt libraries for tests..."
    )
    
    # 复制vcpkg安装的Protobuf和GoogleTest DLL（仅动态链接时需要）
    if(VCPKG_INSTALLED_DIR AND NOT VCPKG_TARGET_TRIPLET MATCHES "static")
        set(VCPKG_BIN_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
        if(EXISTS "${VCPKG_BIN_DIR}/libprotobuf.dll")
            add_custom_command(TARGET flykylin_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${VCPKG_BIN_DIR}/libprotobuf.dll"
                    "$<TARGET_FILE_DIR:flykylin_tests>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${VCPKG_BIN_DIR}/abseil_dll.dll"
                    "$<TARGET_FILE_DIR:flykylin_tests>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${VCPKG_BIN_DIR}/gtest.dll"
                    "$<TARGET_FILE_DIR:flykylin_tests>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${VCPKG_BIN_DIR}/gtest_main.dll"
                    "$<TARGET_FILE_DIR:flykylin_tests>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${VCPKG_BIN_DIR}/gmock.dll"
                    "$<TARGET_FILE_DIR:flykylin_tests>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${VCPKG_BIN_DIR}/gmock_main.dll"
                    "$<TARGET_FILE_DIR:flykylin_tests>"
                COMMENT "Deploying Protobuf and GoogleTest libraries for tests..."
                COMMAND_EXPAND_LISTS
            )
        endif()
    endif()
endif()

# 注册测试（禁用自动发现以避免POST_BUILD阶段DLL依赖问题）
# gtest_discover_tests(flykylin_tests)
add_test(NAME flykylin_tests COMMAND flykylin_tests)
