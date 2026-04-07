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
