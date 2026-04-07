cmake_minimum_required(VERSION 3.15)
project(logic_service)

# 搜集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/proto
)

# 链接公共库和线程库
target_link_libraries(${PROJECT_NAME} PRIVATE chat_common Threads::Threads)

# 如果开启了 proto 的 gRPC 生成，则需要链接 proto 库 (由于沙盒环境暂未开启，先注释)
# target_link_libraries(${PROJECT_NAME} PRIVATE chat_proto)

if(BUILD_TESTING)
    add_executable(logic_test test/logic_test.cpp src/chat_handler.cpp src/message_processor.cpp src/sensitive_filter.cpp)
    target_include_directories(logic_test PRIVATE src ${CMAKE_SOURCE_DIR}/proto)
    target_link_libraries(logic_test PRIVATE chat_common Threads::Threads)
    add_test(NAME LogicTest COMMAND logic_test)
endif()
