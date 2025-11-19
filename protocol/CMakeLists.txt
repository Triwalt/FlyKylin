# Protocol library for Protobuf generated code

find_package(Protobuf CONFIG REQUIRED)

# Generate C++ code from .proto files
set(PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/messages.proto
)

# Use modern protobuf_generate
add_library(flykylin_protocol STATIC ${PROTO_FILES})

# Protobuf已经包含了Abseil依赖，但需要显式链接
target_link_libraries(flykylin_protocol 
    PUBLIC 
        protobuf::libprotobuf
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
