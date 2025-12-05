# Core模块CMakeLists.txt

set(CORE_SOURCES
    # Config模块
    config/UserProfile.cpp
    config/UserProfile.h
    config/ConfigManager.cpp
    config/ConfigManager.h
    
    # Models模块
    models/PeerNode.cpp
    models/PeerNode.h
    models/Message.cpp
    models/Message.h
    
    # Adapters模块
    adapters/ProtobufSerializer.cpp
    adapters/ProtobufSerializer.h

    # Database模块
    database/DatabaseService.cpp
    database/DatabaseService.h
    
    # Communication模块
    communication/PeerDiscovery.cpp
    communication/PeerDiscovery.h
    communication/NetworkInterfaceCache.cpp
    communication/NetworkInterfaceCache.h
    communication/RetryStrategy.cpp
    communication/RetryStrategy.h
    communication/TcpConnection.cpp
    communication/TcpConnection.h
    communication/TcpServer.cpp
    communication/TcpServer.h
    communication/MessageQueue.cpp
    communication/MessageQueue.h
    communication/TcpConnectionManager.cpp
    communication/TcpConnectionManager.h
    
    # Services模块
    services/MessageService.cpp
    services/MessageService.h
    services/LocalEchoService.cpp
    services/LocalEchoService.h
    services/FileTransferService.cpp
    services/FileTransferService.h
    services/ChatSearchService.cpp
    services/ChatSearchService.h
    services/GroupChatManager.cpp
    services/GroupChatManager.h

    # AI模块
    ai/TextEmbeddingEngine.cpp
    ai/TextEmbeddingEngine.h
    ai/NSFWDetector.cpp
    ai/NSFWDetector.h
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
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Gui
    flykylin_protocol
)

option(FLYKYLIN_ENABLE_ONNXRUNTIME "Enable ONNX Runtime based semantic search" ON)

if(FLYKYLIN_ENABLE_ONNXRUNTIME AND ONNXRUNTIME_FOUND)
    if(EXISTS "${ONNXRUNTIME_ROOT}/include/onnxruntime_cxx_api.h")
        target_compile_definitions(flykylin_core PRIVATE FLYKYLIN_ENABLE_ONNXRUNTIME=1)
        target_include_directories(flykylin_core PRIVATE "${ONNXRUNTIME_ROOT}/include")
        # 链接库名称由顶层 CMake �?link_directories(${ONNXRUNTIME_ROOT}/lib) 提供搜索路径
        target_link_libraries(flykylin_core PRIVATE onnxruntime)
    endif()
endif()

if(IS_RK3566 AND RKNPU_FOUND)
    target_compile_definitions(flykylin_core PRIVATE FLYKYLIN_ENABLE_RKNN=1)
    # 链接库名称由顶层 CMake �?link_directories(${RKNPU_ROOT}) 提供搜索路径
    target_link_libraries(flykylin_core PRIVATE rknnrt)
endif()

# Protobuf链接（等待安装）
# if(Protobuf_FOUND)
#     target_link_libraries(flykylin_core PRIVATE protobuf::libprotobuf)
#     target_include_directories(flykylin_core PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# endif()
