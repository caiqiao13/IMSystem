#!/bin/bash
set -e

COMMON_DIR="/workspace/chat-system/common"

# 1. CMakeLists.txt
cat << 'EOF' > ${COMMON_DIR}/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(chat_common)

# 搜集所有源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 添加公共静态库
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# 暴露给外部使用的头文件路径
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 依赖链接配置示例（如依赖 spdlog/hiredis 等时可取消注释）
# target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)
EOF

# 2. Logger (spdlog wrapper placeholder)
cat << 'EOF' > ${COMMON_DIR}/src/logger/logger.h
#pragma once
#include <string>

namespace chat::common {
class Logger {
public:
    static Logger& GetInstance();
    void Init(const std::string& log_file, const std::string& level);
    
    void Debug(const std::string& msg);
    void Info(const std::string& msg);
    void Warn(const std::string& msg);
    void Error(const std::string& msg);

private:
    Logger() = default;
    ~Logger() = default;
};
} // namespace chat::common

#define LOG_DEBUG(msg) chat::common::Logger::GetInstance().Debug(msg)
#define LOG_INFO(msg)  chat::common::Logger::GetInstance().Info(msg)
#define LOG_WARN(msg)  chat::common::Logger::GetInstance().Warn(msg)
#define LOG_ERROR(msg) chat::common::Logger::GetInstance().Error(msg)
EOF

cat << 'EOF' > ${COMMON_DIR}/src/logger/logger.cpp
#include "logger/logger.h"
#include <iostream>

namespace chat::common {
Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::Init(const std::string& log_file, const std::string& level) {
    // TODO: 接入实际的 spdlog 初始化逻辑
    std::cout << "[Logger] Initialized with file: " << log_file << ", level: " << level << std::endl;
}

void Logger::Debug(const std::string& msg) { std::cout << "[DEBUG] " << msg << std::endl; }
void Logger::Info(const std::string& msg)  { std::cout << "[INFO]  " << msg << std::endl; }
void Logger::Warn(const std::string& msg)  { std::cout << "[WARN]  " << msg << std::endl; }
void Logger::Error(const std::string& msg) { std::cerr << "[ERROR] " << msg << std::endl; }

} // namespace chat::common
EOF

# 3. Utils (Time and String)
cat << 'EOF' > ${COMMON_DIR}/src/utils/time_utils.h
#pragma once
#include <string>
#include <cstdint>

namespace chat::common::utils {
class TimeUtils {
public:
    // 获取当前时间戳(毫秒)
    static int64_t GetCurrentTimestampMs();
    // 获取当前时间戳(秒)
    static int64_t GetCurrentTimestampSec();
    // 获取格式化的时间字符串: YYYY-MM-DD HH:MM:SS
    static std::string GetCurrentTimeString();
};
} // namespace chat::common::utils
EOF

cat << 'EOF' > ${COMMON_DIR}/src/utils/time_utils.cpp
#include "utils/time_utils.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace chat::common::utils {

int64_t TimeUtils::GetCurrentTimestampMs() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

int64_t TimeUtils::GetCurrentTimestampSec() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

std::string TimeUtils::GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace chat::common::utils
EOF

cat << 'EOF' > ${COMMON_DIR}/src/utils/string_utils.h
#pragma once
#include <string>
#include <vector>

namespace chat::common::utils {
class StringUtils {
public:
    // 字符串分割
    static std::vector<std::string> Split(const std::string& str, const std::string& delimiter);
    // 去除两端空白字符
    static std::string Trim(const std::string& str);
};
} // namespace chat::common::utils
EOF

cat << 'EOF' > ${COMMON_DIR}/src/utils/string_utils.cpp
#include "utils/string_utils.h"

namespace chat::common::utils {

std::vector<std::string> StringUtils::Split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delimiter, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delimiter.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

std::string StringUtils::Trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

} // namespace chat::common::utils
EOF

# 4. Config Manager
cat << 'EOF' > ${COMMON_DIR}/src/config/config_manager.h
#pragma once
#include <string>
#include <unordered_map>

namespace chat::common {
class ConfigManager {
public:
    static ConfigManager& GetInstance();
    
    // 加载配置文件(yaml/json)
    bool Load(const std::string& file_path);
    
    std::string GetString(const std::string& key, const std::string& default_val = "");
    int GetInt(const std::string& key, int default_val = 0);
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    std::unordered_map<std::string, std::string> config_map_;
};
} // namespace chat::common
EOF

cat << 'EOF' > ${COMMON_DIR}/src/config/config_manager.cpp
#include "config/config_manager.h"
#include "logger/logger.h"

namespace chat::common {

ConfigManager& ConfigManager::GetInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::Load(const std::string& file_path) {
    // TODO: 使用 yaml-cpp 解析实际文件
    LOG_INFO("Loading config from: " + file_path);
    // 模拟读取
    config_map_["server.port"] = "8080";
    config_map_["redis.host"] = "127.0.0.1";
    return true;
}

std::string ConfigManager::GetString(const std::string& key, const std::string& default_val) {
    auto it = config_map_.find(key);
    return (it != config_map_.end()) ? it->second : default_val;
}

int ConfigManager::GetInt(const std::string& key, int default_val) {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return default_val;
        }
    }
    return default_val;
}

} // namespace chat::common
EOF

# 5. Redis Client Stub
cat << 'EOF' > ${COMMON_DIR}/src/redis/redis_client.h
#pragma once
#include <string>

namespace chat::common {
class RedisClient {
public:
    RedisClient();
    ~RedisClient();
    
    bool Connect(const std::string& host, int port);
    bool Set(const std::string& key, const std::string& value, int expire_seconds = 0);
    std::string Get(const std::string& key);
    bool Delete(const std::string& key);
};
} // namespace chat::common
EOF

cat << 'EOF' > ${COMMON_DIR}/src/redis/redis_client.cpp
#include "redis/redis_client.h"
#include "logger/logger.h"

namespace chat::common {

RedisClient::RedisClient() {}
RedisClient::~RedisClient() {}

bool RedisClient::Connect(const std::string& host, int port) {
    LOG_INFO("Connecting to Redis at " + host + ":" + std::to_string(port));
    // TODO: 使用 hiredis 或 redis++ 实现
    return true;
}

bool RedisClient::Set(const std::string& key, const std::string& value, int expire_seconds) {
    LOG_DEBUG("Redis SET key: " + key);
    return true;
}

std::string RedisClient::Get(const std::string& key) {
    LOG_DEBUG("Redis GET key: " + key);
    return "";
}

bool RedisClient::Delete(const std::string& key) {
    LOG_DEBUG("Redis DEL key: " + key);
    return true;
}

} // namespace chat::common
EOF

# 6. RabbitMQ Client Stub
cat << 'EOF' > ${COMMON_DIR}/src/mq/rabbitmq_client.h
#pragma once
#include <string>
#include <functional>

namespace chat::common {
using MessageCallback = std::function<void(const std::string&)>;

class RabbitMQClient {
public:
    RabbitMQClient();
    ~RabbitMQClient();
    
    bool Connect(const std::string& host, int port, const std::string& user, const std::string& pass);
    bool Publish(const std::string& exchange, const std::string& routing_key, const std::string& message);
    bool Subscribe(const std::string& queue_name, MessageCallback callback);
};
} // namespace chat::common
EOF

cat << 'EOF' > ${COMMON_DIR}/src/mq/rabbitmq_client.cpp
#include "mq/rabbitmq_client.h"
#include "logger/logger.h"

namespace chat::common {

RabbitMQClient::RabbitMQClient() {}
RabbitMQClient::~RabbitMQClient() {}

bool RabbitMQClient::Connect(const std::string& host, int port, const std::string& user, const std::string& pass) {
    LOG_INFO("Connecting to RabbitMQ at " + host + ":" + std::to_string(port));
    // TODO: 使用 rabbitmq-c 库实现连接
    return true;
}

bool RabbitMQClient::Publish(const std::string& exchange, const std::string& routing_key, const std::string& message) {
    LOG_DEBUG("RabbitMQ Publish to exchange: " + exchange + ", routing_key: " + routing_key);
    return true;
}

bool RabbitMQClient::Subscribe(const std::string& queue_name, MessageCallback callback) {
    LOG_INFO("RabbitMQ Subscribe to queue: " + queue_name);
    return true;
}

} // namespace chat::common
EOF

echo "All common library files generated successfully."
