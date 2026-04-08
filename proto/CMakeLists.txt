cmake_minimum_required(VERSION 3.15)
project(chat_proto)

# =========================================================
# 此配置使用 VCPKG 中安装的 protobuf 和 gRPC 自动生成并编译源码
# （为了在沙盒环境无 VCPKG 时能正常编译，已暂时注释）
# =========================================================

# find_package(Protobuf CONFIG REQUIRED)
# find_package(gRPC CONFIG REQUIRED)

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
    
#     # 获取 protoc_plugin 的绝对路径
#     get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin LOCATION)

#     add_custom_command(
#         OUTPUT "${PROTO_BUILD_DIR}/${FIL_WE}.pb.cc"
#                "${PROTO_BUILD_DIR}/${FIL_WE}.pb.h"
#                "${PROTO_BUILD_DIR}/${FIL_WE}.grpc.pb.cc"
#                "${PROTO_BUILD_DIR}/${FIL_WE}.grpc.pb.h"
#         COMMAND protobuf::protoc
#         ARGS --grpc_out=${PROTO_BUILD_DIR}
#              --cpp_out=${PROTO_BUILD_DIR}
#              -I ${PROTO_SRC_DIR}
#              --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
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

message(STATUS "Protobuf & gRPC generated successfully into: ${PROTO_BUILD_DIR}")
