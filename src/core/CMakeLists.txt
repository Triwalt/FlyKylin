# Core模块CMakeLists.txt

set(CORE_SOURCES
    models/PeerNode.cpp
    models/PeerNode.h
    communication/PeerDiscovery.cpp
    communication/PeerDiscovery.h
)

# 暂时禁用Protobuf（等待安装）
# find_package(Protobuf REQUIRED)
# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS protobuf/message.proto)
# list(APPEND CORE_SOURCES ${PROTO_SRCS} ${PROTO_HDRS})

add_library(flykylin_core STATIC ${CORE_SOURCES})

target_include_directories(flykylin_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(flykylin_core PUBLIC
    Qt6::Core
    Qt6::Network
)

# Protobuf链接（等待安装）
# if(Protobuf_FOUND)
#     target_link_libraries(flykylin_core PRIVATE protobuf::libprotobuf)
#     target_include_directories(flykylin_core PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# endif()
