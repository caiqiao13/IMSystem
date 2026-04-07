#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include "logger/logger.h"
#include "sensitive_filter.h"
#include "message_processor.h"
#include "utils/time_utils.h"

using namespace chat::logic;

// 模拟 gRPC Server 的启动流程
// 在真实的 gRPC 环境下，这里会注册 `ChatServiceImpl` 并调用 `builder.BuildAndStart()`
void StartGrpcServer() {
    std::string server_address("0.0.0.0:50051");
    LOG_INFO("Logic Service listening on " + server_address);

    // 阻塞线程模拟服务器运行
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(24));
    }
}

int main() {
    // 1. 初始化日志
    chat::common::Logger::GetInstance().Init("logic.log", "info");
    LOG_INFO("Starting Chat System Logic Service...");

    // 2. 初始化敏感词库
    std::vector<std::string> bad_words = { "枪支", "毒品", "政治敏感" };
    SensitiveFilter::GetInstance().Init(bad_words);

    // 3. 模拟一条请求 (相当于接收到了 Gateway 通过 gRPC 发来的 SendMsgReq)
    std::thread simulator([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        LOG_INFO("Simulating an incoming message from Gateway...");
        
        LogicChatMsg mock_msg;
        mock_msg.msg_id = "MSG-UUID-12345678";
        mock_msg.sender_id = 1001;
        mock_msg.receiver_id = 2002;
        mock_msg.session_type = 1; // 单聊
        mock_msg.msg_type = 1;     // 文本
        mock_msg.content = "我要买枪支和子弹"; // 包含敏感词
        mock_msg.send_time = chat::common::utils::TimeUtils::GetCurrentTimestampMs();

        std::string final_seq_id;
        bool success = MessageProcessor::GetInstance().ProcessIncomingMessage(mock_msg, final_seq_id);
        
        if (success) {
            LOG_INFO("Simulation Success. Filtered Content: [" + mock_msg.content + "], Assigned SeqId: " + final_seq_id);
        } else {
            LOG_ERROR("Simulation Failed.");
        }
    });

    // 4. 启动 gRPC 逻辑服务主循环
    try {
        StartGrpcServer();
    } catch (const std::exception& e) {
        LOG_ERROR("Logic Service Exception: " + std::string(e.what()));
        return -1;
    }

    simulator.join();
    return 0;
}
