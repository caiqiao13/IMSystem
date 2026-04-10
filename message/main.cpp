#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include "logger/logger.h"
#include "config/config_manager.h"
#include "message_sync.h"
#include "utils/time_utils.h"

using namespace chat::message;
using namespace chat::logic; // 借用 LogicChatMsg 的定义结构

// 模拟 Message gRPC 服务端的启动
void StartMessageGrpcServer() {
    int port = chat::common::ConfigManager::GetInstance().GetInt("server.grpc_port", 50052);
    std::string server_address("0.0.0.0:" + std::to_string(port));
    LOG_INFO("Message Service listening on " + server_address);

    // 阻塞线程模拟服务器运行
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(24));
    }
}

int main() {
    // 1) 加载配置
    const std::string config_path = "message/config/message_config.yaml";
    if (!chat::common::ConfigManager::GetInstance().Load(config_path)) {
        chat::common::Logger::GetInstance().Init("logs/message.log", "info");
        LOG_ERROR("Failed to load config: " + config_path);
        return -1;
    }

    // 2) 初始化日志
    const std::string log_file = chat::common::ConfigManager::GetInstance().GetString("log.file", "logs/message.log");
    const std::string log_level = chat::common::ConfigManager::GetInstance().GetString("log.level", "info");
    chat::common::Logger::GetInstance().Init(log_file, log_level);
    LOG_INFO("Starting Chat System Message Service...");

    // 3. 初始化核心同步服务 (连接 MySQL 与 MongoDB)
    if (!MessageSync::GetInstance().Init()) {
        LOG_ERROR("Failed to initialize MessageSync (DB connections failed)");
        return -1;
    }

    // 4. 模拟接收上游 Logic Server 的 SaveMsgReq
    std::thread save_simulator([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        LOG_INFO("Simulating [SaveMessage] request from Logic Service...");
        LogicChatMsg mock_msg;
        mock_msg.msg_id = "MSG-UUID-SAVE-001";
        mock_msg.sender_id = 1001;
        mock_msg.receiver_id = 2002;
        mock_msg.session_type = 1;
        mock_msg.msg_type = 1;
        mock_msg.content = "Hello! This is a test message to be saved.";
        
        std::string new_seq_id = MessageSync::GetInstance().SaveMessage(mock_msg);
        LOG_INFO("SaveMessage Simulation Completed. Allocated SeqID: " + new_seq_id);
    });

    // 5. 模拟接收 Gateway 离线同步请求 PullOfflineMsgReq
    std::thread pull_simulator([]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        LOG_INFO("Simulating [PullOfflineMessages] request from Client/Gateway...");
        uint64_t target_user = 2002;
        std::string client_last_seq = "SEQ_1770000000000_10000"; // 客户端最后收到的序列号
        
        auto messages = MessageSync::GetInstance().PullOfflineMessages(target_user, client_last_seq, 100);
        LOG_INFO("PullOfflineMessages Simulation Completed. Fetched " + std::to_string(messages.size()) + " messages.");
        for (const auto& m : messages) {
            LOG_DEBUG("  -> Fetched MSG_ID: " + m.msg_id + ", Content: " + m.content);
        }
    });

    // 6. 启动 gRPC 服务
    try {
        StartMessageGrpcServer();
    } catch (const std::exception& e) {
        LOG_ERROR("Message Service Exception: " + std::string(e.what()));
        return -1;
    }

    save_simulator.join();
    pull_simulator.join();
    return 0;
}
