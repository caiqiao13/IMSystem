#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

#include "logger/logger.h"
#include "config/config_manager.h"
#include "sensitive_filter.h"
#include "message_processor.h"
#include "utils/time_utils.h"

// 如果能引入生成的 gRPC 代码，则启动真实服务器
#if __has_include("chat.grpc.pb.h")
#include "chat.grpc.pb.h"
#include <grpcpp/grpcpp.h>
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using chat::pb::LogicService;
using chat::pb::SendMsgReq;
using chat::pb::SendMsgResp;

class LogicServiceImpl final : public LogicService::Service {
public:
    Status SendMessage(ServerContext* context, const SendMsgReq* request, SendMsgResp* response) override {
        chat::logic::LogicChatMsg msg;
        msg.msg_id = request->msg_id();
        msg.sender_id = request->sender_id();
        msg.receiver_id = request->receiver_id();
        msg.session_type = request->session_type();
        msg.msg_type = request->msg_type();
        msg.content = request->content();
        msg.send_time = chat::common::utils::TimeUtils::GetCurrentTimestampMs();

        std::string final_seq_id;
        bool success = chat::logic::MessageProcessor::GetInstance().ProcessIncomingMessage(msg, final_seq_id);
        
        if (success) {
            response->set_code(0);
            response->set_msg("SUCCESS");
            response->set_seq_id(final_seq_id);
            return Status::OK;
        } else {
            response->set_code(500);
            response->set_msg("Internal Error or Validation Failed");
            return Status::OK; // 业务错误仍返回 OK 状态码
        }
    }
};

void StartGrpcServer() {
    int port = chat::common::ConfigManager::GetInstance().GetInt("server.grpc_port", 50051);
    std::string server_address("0.0.0.0:" + std::to_string(port));
    LogicServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    
    LOG_INFO("Real Logic gRPC Service listening on " + server_address);
    server->Wait();
}
#else
void StartGrpcServer() {
    int port = chat::common::ConfigManager::GetInstance().GetInt("server.grpc_port", 50051);
    std::string server_address("0.0.0.0:" + std::to_string(port));
    LOG_INFO("Mock Logic Service listening on " + server_address);
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(24));
    }
}
#endif

int main() {
    // 1) 加载配置
    const std::string config_path = "logic/config/logic_config.yaml";
    if (!chat::common::ConfigManager::GetInstance().Load(config_path)) {
        chat::common::Logger::GetInstance().Init("logs/logic.log", "info");
        LOG_ERROR("Failed to load config: " + config_path);
        return -1;
    }

    // 2) 初始化日志
    const std::string log_file = chat::common::ConfigManager::GetInstance().GetString("log.file", "logs/logic.log");
    const std::string log_level = chat::common::ConfigManager::GetInstance().GetString("log.level", "info");
    chat::common::Logger::GetInstance().Init(log_file, log_level);
    LOG_INFO("Starting Chat System Logic Service...");

    // 3. 初始化敏感词库
    auto bad_words = chat::common::ConfigManager::GetInstance().GetStringList("sensitive_words");
    if (bad_words.empty()) {
        bad_words = { "枪支", "毒品", "政治敏感" };
    }
    chat::logic::SensitiveFilter::GetInstance().Init(bad_words);

    // 4. 启动 gRPC 逻辑服务主循环
    try {
        StartGrpcServer();
    } catch (const std::exception& e) {
        LOG_ERROR("Logic Service Exception: " + std::string(e.what()));
        return -1;
    }

    return 0;
}
