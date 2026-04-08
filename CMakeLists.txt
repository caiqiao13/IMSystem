cmake_minimum_required(VERSION 3.15)
project(ChatSystem VERSION 1.0.0 LANGUAGES CXX)

# ==============================================================================
# 1. 编译选项 (Compile Options)
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 根据构建类型设置编译标志
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# 通用编译警告选项
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O3)
endif()

# ==============================================================================
# 2. 设置输出目录 (Output Directories)
# ==============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ==============================================================================
# 3. 查找公共依赖包 (Find Dependencies)
# ==============================================================================
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ==========================================================
# 依赖库查找模板 (由于环境未安装，这里暂时注释掉，按需启用)
# ==========================================================
# Asio (通常 header-only)
# find_package(asio REQUIRED)

# gRPC & Protobuf
# find_package(gRPC REQUIRED)
# find_package(Protobuf REQUIRED)

# Redis (redis++ 和 hiredis)
# find_package(redis++ REQUIRED)

# RabbitMQ
# find_package(rabbitmq-c REQUIRED)

# 日志框架 spdlog
# find_package(spdlog REQUIRED)

# 监控指标 Prometheus
# find_package(prometheus-cpp REQUIRED)

# 关系数据库 MySQL
# find_package(MySQL REQUIRED)

# 文档数据库 MongoDB
# find_package(mongocxx REQUIRED)

# ==============================================================================
# 4. 打印配置信息 (Print Configuration)
# ==============================================================================
message(STATUS "=======================================================")
message(STATUS "Project Name       : ${PROJECT_NAME}")
message(STATUS "Project Version    : ${PROJECT_VERSION}")
message(STATUS "Build Type         : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard       : ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler           : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Output Binaries    : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Output Libraries   : ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "=======================================================")

# ==============================================================================
# 5. 添加子项目 (Add Subdirectories)
# ==============================================================================
add_subdirectory(common)
add_subdirectory(proto)
add_subdirectory(gateway)
add_subdirectory(logic)
add_subdirectory(user)
add_subdirectory(message)
add_subdirectory(push)
add_subdirectory(admin)


# 开启测试支持
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()
    # find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()

# ==============================================================================
# 6. 安装配置 (Install Configuration)
# ==============================================================================
# 设置安装前缀路径（默认为 /usr/local）
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/opt/chat-system" CACHE PATH "Install path prefix" FORCE)
endif()

# (各子项目的库文件或可执行文件将在各自的 CMakeLists.txt 中使用 install 命令)
# 这里先统一安装一些配置文件或脚本
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deploy/ DESTINATION deploy)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/scripts/ DESTINATION scripts)

# ==============================================================================
# 7. 创建安装包配置 (CPack/Packaging)
# ==============================================================================
set(CPACK_PACKAGE_NAME "chat-system")
set(CPACK_PACKAGE_VENDOR "IM Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IM Distributed Microservice System")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

# 支持打出 .deb 和 .rpm 等包格式
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "IM Team") 
set(CPACK_RPM_PACKAGE_LICENSE "MIT")

include(CPack)
