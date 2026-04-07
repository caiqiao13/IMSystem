cmake_minimum_required(VERSION 3.15)
project(message_service)

# 搜集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/proto
    ${CMAKE_SOURCE_DIR}/logic/src # 需要复用 logic 中定义的结构体模拟
)

# 链接公共库和线程库
target_link_libraries(${PROJECT_NAME} PRIVATE chat_common Threads::Threads)

# 如果开启了 proto 的 gRPC 生成，则需要链接 proto 库 (由于沙盒环境暂未开启，先注释)
# target_link_libraries(${PROJECT_NAME} PRIVATE chat_proto)

# 如果连接真实的 MongoDB / MySQL，取消注释下方的库依赖
# find_package(mongocxx REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE mongo::mongocxx_shared)
# target_link_libraries(${PROJECT_NAME} PRIVATE mysqlclient)

if(BUILD_TESTING)
    add_executable(message_test test/message_test.cpp src/mongo_storage.cpp src/mysql_index.cpp src/message_sync.cpp)
    target_include_directories(message_test PRIVATE src ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/logic/src)
    target_link_libraries(message_test PRIVATE chat_common Threads::Threads)
    add_test(NAME MessageTest COMMAND message_test)
endif()
