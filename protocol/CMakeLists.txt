# Protocol library for Protobuf generated code

# Protobuf, absl and utf8_range are discovered in the top-level CMakeLists.
# Here we only define the protocol library and its link dependencies.

# Generate C++ code from .proto files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/messages.proto
)

# Use modern protobuf_generate
add_library(flykylin_protocol STATIC ${PROTO_FILES})

# Protobuf依赖Abseil和utf8_range，这里显式链接所需组件
target_link_libraries(flykylin_protocol 
    PUBLIC 
        protobuf::libprotobuf
        absl::log
        utf8_range::utf8_range
)

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
