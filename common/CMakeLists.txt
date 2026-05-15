cmake_minimum_required(VERSION 3.15)
project(chat_common)

include(FetchContent)
# 依赖由根目录 `cmake/Dependencies.cmake` 统一处理：
# - 优先使用系统 find_package
# - 找不到则 FetchContent 拉取源码构建
# 这样 Windows 本地环境不需要预装 spdlog/jsoncpp 也能配置成功。
# 引入 bcrypt (用于密码加密)
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

# MySQL / MongoDB C++ 驱动（可选；未安装时请通过 vcpkg 安装或自行 target_link_libraries）
if(TARGET unofficial::mysql-connector-cpp::mysqlcppconn)
  target_link_libraries(${PROJECT_NAME} PUBLIC unofficial::mysql-connector-cpp::mysqlcppconn)
else()
  message(WARNING
    "[common] MySQL Connector C++ CMake 目标未找到 (unofficial-mysql-connector-cpp)。"
    " 使用连接池时请安装对应库并通过 vcpkg toolchain 配置工程。")
endif()

if(TARGET mongo::mongocxx_shared)
  target_link_libraries(${PROJECT_NAME} PUBLIC mongo::mongocxx_shared)
elseif(TARGET mongocxx_static)
  target_link_libraries(${PROJECT_NAME} PUBLIC mongocxx_static)
else()
  message(WARNING
    "[common] mongocxx CMake 目标未找到。"
    " 使用 MongoConnectionPool 时请安装 mongo-cxx-driver 并配置 vcpkg / CMAKE_PREFIX_PATH。")
endif()
