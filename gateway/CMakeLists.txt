cmake_minimum_required(VERSION 3.15)
project(gateway_server)

# 引入 ASIO 头文件依赖 (Header-only)
include(FetchContent)
FetchContent_Declare(
  asio
  URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-24-0.tar.gz
)
FetchContent_MakeAvailable(asio)

# 搜集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${asio_SOURCE_DIR}/asio/include
)

# 链接公共库和线程库
target_link_libraries(${PROJECT_NAME} PRIVATE chat_common Threads::Threads)

# 如果宏需要无 Boost 支持的独立 ASIO
target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE)

if(BUILD_TESTING)
    add_executable(gateway_test test/gateway_test.cpp src/gateway_server.cpp src/tcp_server.cpp src/http_server.cpp)
    target_include_directories(gateway_test PRIVATE src ${asio_SOURCE_DIR}/asio/include)
    target_link_libraries(gateway_test PRIVATE chat_common Threads::Threads)
    target_compile_definitions(gateway_test PRIVATE ASIO_STANDALONE)
    add_test(NAME GatewayTest COMMAND gateway_test)
endif()
