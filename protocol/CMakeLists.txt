# Protocol library for Protobuf generated code

# Protobuf, absl and utf8_range are discovered in the top-level CMakeLists.
# Here we only define the protocol library and its link dependencies.

# Generate C++ code from .proto files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/messages.proto
)

# Use modern protobuf_generate
add_library(flykylin_protocol STATIC ${PROTO_FILES})

# Protobuf依赖Abseil和utf8_range，这里在存在 CONFIG 目标时显式链接；
# 在仅提供系统模块包的环境（如Linux/WSL）下退回到 Protobuf_LIBRARIES。
if(TARGET protobuf::libprotobuf)
    target_link_libraries(flykylin_protocol
        PUBLIC
            protobuf::libprotobuf
    )
    if(TARGET absl::log)
        target_link_libraries(flykylin_protocol PUBLIC absl::log)
    endif()
    if(TARGET utf8_range::utf8_range)
        target_link_libraries(flykylin_protocol PUBLIC utf8_range::utf8_range)
    endif()
elseif(Protobuf_LIBRARIES)
    target_link_libraries(flykylin_protocol PUBLIC ${Protobuf_LIBRARIES})
endif()

target_include_directories(flykylin_protocol
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate protobuf code
protobuf_generate(
    TARGET flykylin_protocol
    LANGUAGE cpp
    PROTOS ${PROTO_FILES}
)
