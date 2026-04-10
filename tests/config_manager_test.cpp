#include "config/config_manager.h"

#include <iostream>

int main() {
    const std::string path = "gateway/config/gateway_config.yaml";
    if (!chat::common::ConfigManager::GetInstance().Load(path)) {
        std::cerr << "ConfigManagerTest: failed to load " << path << std::endl;
        return 1;
    }

    const int tcp_port = chat::common::ConfigManager::GetInstance().GetInt("server.tcp_port", -1);
    const int http_port = chat::common::ConfigManager::GetInstance().GetInt("server.http_port", -1);
    const std::string log_level = chat::common::ConfigManager::GetInstance().GetString("log.level", "");

    if (tcp_port != 9000) {
        std::cerr << "Expected server.tcp_port=9000, got " << tcp_port << std::endl;
        return 2;
    }
    if (http_port != 8080) {
        std::cerr << "Expected server.http_port=8080, got " << http_port << std::endl;
        return 3;
    }
    if (log_level.empty()) {
        std::cerr << "Expected non-empty log.level" << std::endl;
        return 4;
    }

    auto empty = chat::common::ConfigManager::GetInstance().GetStringList("this_key_should_not_exist");
    if (!empty.empty()) {
        std::cerr << "Expected empty list for missing key prefix" << std::endl;
        return 5;
    }

    return 0;
}

