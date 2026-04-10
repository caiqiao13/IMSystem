#include <iostream>
#include <thread>
#include <chrono>

#include "logger/logger.h"
#include "config/config_manager.h"
#include "token_manager.h"
#include "auth_manager.h"
#include "friend_manager.h"

using namespace chat::user;

// 模拟 User gRPC 服务的启动
void StartUserGrpcServer() {
    int port = chat::common::ConfigManager::GetInstance().GetInt("server.grpc_port", 50050);
    std::string server_address("0.0.0.0:" + std::to_string(port));
    LOG_INFO("User Service listening on " + server_address);

    // 阻塞线程模拟 gRPC 服务器的长期运行
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(24));
    }
}

int main() {
    // 1) 加载配置
    const std::string config_path = "user/config/user_config.yaml";
    if (!chat::common::ConfigManager::GetInstance().Load(config_path)) {
        chat::common::Logger::GetInstance().Init("logs/user.log", "info");
        LOG_ERROR("Failed to load config: " + config_path);
        return -1;
    }

    // 2) 初始化日志
    const std::string log_file = chat::common::ConfigManager::GetInstance().GetString("log.file", "logs/user.log");
    const std::string log_level = chat::common::ConfigManager::GetInstance().GetString("log.level", "info");
    chat::common::Logger::GetInstance().Init(log_file, log_level);
    LOG_INFO("Starting Chat System User Service...");

    // 3. 初始化 Token 管理器 (连接 Redis)
    const std::string redis_host = chat::common::ConfigManager::GetInstance().GetString("dependencies.redis.host", "127.0.0.1");
    const int redis_port = chat::common::ConfigManager::GetInstance().GetInt("dependencies.redis.port", 6379);
    if (!TokenManager::GetInstance().Init(redis_host, redis_port)) {
        LOG_ERROR("Failed to initialize TokenManager (Redis connection failed)");
        return -1;
    }

    // 4. 模拟 Gateway 转发来的客户端登录请求
    std::thread auth_simulator([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        LOG_INFO("Simulating [Login] request...");
        auto [success, msg, token, user_info] = AuthManager::GetInstance().Login("test_user", "123456", 1); // PC 登录
        
        if (success) {
            LOG_INFO("Simulation Login Success! Token: " + token + ", UserInfo: " + user_info);
            
            // 登录成功后，模拟客户端拉取好友列表
            std::this_thread::sleep_for(std::chrono::seconds(1));
            LOG_INFO("Simulating [GetFriendList] request with Token...");
            
            // a. 鉴权
            if (TokenManager::GetInstance().VerifyToken(1001, token)) {
                // b. 查业务
                int total = 0;
                auto friends = FriendManager::GetInstance().GetFriendList(1001, 1, 20, total);
                LOG_INFO("Simulation GetFriendList Success! Found " + std::to_string(total) + " friends.");
            } else {
                LOG_ERROR("Simulation GetFriendList Failed. Token invalid.");
            }
        } else {
            LOG_ERROR("Simulation Login Failed: " + msg);
        }
    });

    // 5. 启动 gRPC 服务
    try {
        StartUserGrpcServer();
    } catch (const std::exception& e) {
        LOG_ERROR("User Service Exception: " + std::string(e.what()));
        return -1;
    }

    auth_simulator.join();
    return 0;
}
