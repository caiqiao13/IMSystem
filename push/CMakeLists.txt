cmake_minimum_required(VERSION 3.15)
project(push_service)

# 搜集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# 添加包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/proto
    ${CMAKE_SOURCE_DIR}/common/src # 需要复用 common 里的 RabbitMQ 客户端和日志
)

# 链接公共库和线程库
target_link_libraries(${PROJECT_NAME} PRIVATE chat_common Threads::Threads)

# 如果开启了 proto 的 gRPC 生成，则需要链接 proto 库 (由于沙盒环境暂未开启，先注释)
# target_link_libraries(${PROJECT_NAME} PRIVATE chat_proto)

# APNs / FCM 需要 HTTP/2 客户端，比如 libcurl
# find_package(CURL REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)

if(BUILD_TESTING)
    add_executable(push_test test/push_test.cpp src/apns_client.cpp src/fcm_client.cpp src/push_manager.cpp)
    target_include_directories(push_test PRIVATE src ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/common/src)
    target_link_libraries(push_test PRIVATE chat_common Threads::Threads)
    add_test(NAME PushTest COMMAND push_test)
endif()
