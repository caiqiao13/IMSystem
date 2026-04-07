#pragma once
#include "chat_handler.h"
#include <string>
#include <memory>

namespace chat::logic {

class MessageProcessor {
public:
    static MessageProcessor& GetInstance();
    
    // 主处理入口：客户端发来消息
    bool ProcessIncomingMessage(LogicChatMsg& msg, std::string& out_seq_id);
    
private:
    MessageProcessor() = default;
    ~MessageProcessor() = default;

    std::unique_ptr<BaseChatHandler> GetHandler(int session_type);

    // 调用下游 Message Server 进行持久化并生成严格递增 seq_id
    bool CallMessageServiceToSave(LogicChatMsg& msg, std::string& out_seq_id);
    
    // 发送消息到 Redis PubSub 或 RabbitMQ 进行推送分发
    bool DispatchToPushService(const LogicChatMsg& msg);
};

} // namespace chat::logic
