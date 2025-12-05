# Protocol library for Protobuf generated code

# Protobuf, absl and utf8_range are discovered in the top-level CMakeLists.
# Here we only define the protocol library and its link dependencies.

# Generate C++ code from .proto files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/messages.proto
)

# Use modern protobuf_generate
add_library(flykylin_protocol STATIC ${PROTO_FILES})

# Protobuf依赖Abseil和utf8_range：
# - 在 Windows/vcpkg 环境下使用 CONFIG 包提供的 protobuf::libprotobuf 目标；
# - 在 Linux/RK3566 交叉编译等仅存在模块包的环境下，统一使用 Protobuf_LIBRARIES，
#   避免依赖可能缺少 IMPORTED_LOCATION 的 imported target。
if(WIN32 AND TARGET protobuf::libprotobuf)
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
