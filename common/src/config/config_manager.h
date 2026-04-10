#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace chat::common {
class ConfigManager {
public:
    static ConfigManager& GetInstance();
    
    // 加载配置文件(yaml/json)
    bool Load(const std::string& file_path);
    
    std::string GetString(const std::string& key, const std::string& default_val = "");
    int GetInt(const std::string& key, int default_val = 0);
    bool GetBool(const std::string& key, bool default_val = false);
    std::vector<std::string> GetStringList(const std::string& key_prefix);
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    std::unordered_map<std::string, std::string> config_map_;
};
} // namespace chat::common
