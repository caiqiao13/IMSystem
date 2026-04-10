cmake_minimum_required(VERSION 3.15)
project(admin_server)

# 引入 ASIO
find_package(asio CONFIG REQUIRED)

# 搜集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接公共库和线程库
target_link_libraries(${PROJECT_NAME} PRIVATE chat_common Threads::Threads asio::asio)

# 独立 ASIO
target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE)

if(BUILD_TESTING)
    add_executable(admin_test test/admin_test.cpp src/admin_server.cpp src/api_handler.cpp)
    target_include_directories(admin_test PRIVATE src)
    target_link_libraries(admin_test PRIVATE chat_common Threads::Threads asio::asio)
    target_compile_definitions(admin_test PRIVATE ASIO_STANDALONE)
    add_test(NAME AdminTest COMMAND admin_test)
endif()
