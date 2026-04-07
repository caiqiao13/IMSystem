cmake_minimum_required(VERSION 3.15)
project(chat_common)

include(FetchContent)

# 1. 引入 spdlog
FetchContent_Declare(
  spdlog
  URL https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.tar.gz
)
FetchContent_MakeAvailable(spdlog)

# 2. 引入 jsoncpp
FetchContent_Declare(
  jsoncpp
  URL https://github.com/open-source-parsers/jsoncpp/archive/refs/tags/1.9.5.tar.gz
)
set(JSONCPP_WITH_TESTS OFF CACHE BOOL "Disable jsoncpp tests" FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "Disable jsoncpp tests" FORCE)
FetchContent_MakeAvailable(jsoncpp)

# 3. 引入 bcrypt (用于密码加密)
FetchContent_Declare(
  bcrypt
  GIT_REPOSITORY https://github.com/trusch/libbcrypt.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(bcrypt)

# 搜集所有源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 添加公共静态库
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# 暴露给外部使用的头文件路径
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${bcrypt_SOURCE_DIR}/include
)

# 链接 spdlog 和 jsoncpp 库，这样其他依赖 common 的微服务会自动继承这两个库
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog jsoncpp_lib bcrypt)
