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
)

target_include_directories(flykylin_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
)

# 注册测试
gtest_discover_tests(flykylin_tests)
