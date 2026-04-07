cmake_minimum_required(VERSION 3.15)
project(user_service)

# 搜集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/proto
    ${CMAKE_SOURCE_DIR}/common/src # 依赖 common 库中的 Redis 客户端与工具类
)

# 链接公共库和线程库
target_link_libraries(${PROJECT_NAME} PRIVATE chat_common Threads::Threads)

# 如果开启了 proto 的 gRPC 生成，则需要链接 proto 库 (由于沙盒环境暂未开启，先注释)
# target_link_libraries(${PROJECT_NAME} PRIVATE chat_proto)

# 如果连接真实的 MySQL/Redis，可以取消注释下方的库依赖
# target_link_libraries(${PROJECT_NAME} PRIVATE mysqlclient)
# target_link_libraries(${PROJECT_NAME} PRIVATE hiredis redis++)

if(BUILD_TESTING)
    add_executable(user_test test/user_test.cpp src/auth_manager.cpp src/friend_manager.cpp src/token_manager.cpp)
    target_include_directories(user_test PRIVATE src ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/common/src)
    target_link_libraries(user_test PRIVATE chat_common Threads::Threads)
    add_test(NAME UserTest COMMAND user_test)
endif()
