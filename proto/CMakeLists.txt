cmake_minimum_required(VERSION 3.15)
project(chat_proto)

# =========================================================
# 此处配置由于缺少环境中的 protobuf/grpc, 先仅做占位和打印
# 当实际环境中安装了 protobuf-compiler 和 grpc_cpp_plugin 后
# 取消注释即可实现自动生成 C++ 源码并编译为静态库
# =========================================================

# find_package(Protobuf REQUIRED)
# find_package(gRPC REQUIRED)

# set(PROTO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# set(PROTO_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
# file(MAKE_DIRECTORY ${PROTO_BUILD_DIR})

# file(GLOB PROTO_FILES "${PROTO_SRC_DIR}/*.proto")

# set(PROTO_SRCS)
# set(PROTO_HDRS)
# set(GRPC_SRCS)
# set(GRPC_HDRS)

# foreach(FIL ${PROTO_FILES})
#     get_filename_component(FIL_WE ${FIL} NAME_WE)
#     list(APPEND PROTO_SRCS "${PROTO_BUILD_DIR}/${FIL_WE}.pb.cc")
#     list(APPEND PROTO_HDRS "${PROTO_BUILD_DIR}/${FIL_WE}.pb.h")
#     list(APPEND GRPC_SRCS "${PROTO_BUILD_DIR}/${FIL_WE}.grpc.pb.cc")
#     list(APPEND GRPC_HDRS "${PROTO_BUILD_DIR}/${FIL_WE}.grpc.pb.h")
    
#     add_custom_command(
#         OUTPUT "${PROTO_BUILD_DIR}/${FIL_WE}.pb.cc"
#                "${PROTO_BUILD_DIR}/${FIL_WE}.pb.h"
#                "${PROTO_BUILD_DIR}/${FIL_WE}.grpc.pb.cc"
#                "${PROTO_BUILD_DIR}/${FIL_WE}.grpc.pb.h"
#         COMMAND ${Protobuf_PROTOC_EXECUTABLE}
#         ARGS --grpc_out=${PROTO_BUILD_DIR}
#              --cpp_out=${PROTO_BUILD_DIR}
#              -I ${PROTO_SRC_DIR}
#              --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
#              ${FIL}
#         DEPENDS ${FIL}
#     )
# endforeach()

# add_library(${PROJECT_NAME} STATIC ${PROTO_SRCS} ${GRPC_SRCS})

# target_include_directories(${PROJECT_NAME} PUBLIC 
#     $<BUILD_INTERFACE:${PROTO_BUILD_DIR}>
#     $<INSTALL_INTERFACE:include>
# )

# target_link_libraries(${PROJECT_NAME} PUBLIC
#     protobuf::libprotobuf
#     gRPC::grpc++
# )

message(STATUS "Proto directory configured. (Uncomment build rules in proto/CMakeLists.txt when Protobuf/gRPC is installed)")
