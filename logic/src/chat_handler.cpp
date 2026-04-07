#include "chat_handler.h"
#include "logger/logger.h"
#include "utils/time_utils.h"

namespace chat::logic {

// =====================================
// 单聊处理器
// =====================================
bool SingleChatHandler::Validate(const LogicChatMsg& msg) {
    if (msg.sender_id == msg.receiver_id) {
        LOG_WARN("Cannot send message to self: " + std::to_string(msg.sender_id));
        return false;
    }
    
    // TODO: 调用 User 服务 RPC 校验发送者和接收者是否是好友关系
    LOG_INFO("Validating single chat: " + std::to_string(msg.sender_id) + " -> " + std::to_string(msg.receiver_id));
    return true; // 假设校验通过
}

bool SingleChatHandler::ProcessSend(LogicChatMsg& msg) {
    if (msg.msg_type == 1) { // 文本消息需要过敏感词
        if (SensitiveFilter::GetInstance().HasSensitiveWord(msg.content)) {
            LOG_WARN("Sensitive word detected in single chat msg: " + msg.msg_id);
            msg.content = SensitiveFilter::GetInstance().Filter(msg.content);
        }
    }
    
    // 补齐服务端数据
    msg.server_time = chat::common::utils::TimeUtils::GetCurrentTimestampMs();
    
    LOG_INFO("Single chat processed, msg_id: " + msg.msg_id);
    return true;
}

// =====================================
// 群聊处理器
// =====================================
bool GroupChatHandler::Validate(const LogicChatMsg& msg) {
    if (msg.receiver_id == 0) {
        LOG_WARN("Invalid group ID: " + std::to_string(msg.receiver_id));
        return false;
    }
    
    // TODO: 调用 User/Group 服务 RPC 校验发送者是否在群内、是否被禁言
    LOG_INFO("Validating group chat: " + std::to_string(msg.sender_id) + " -> group " + std::to_string(msg.receiver_id));
    return true; // 假设校验通过
}

bool GroupChatHandler::ProcessSend(LogicChatMsg& msg) {
    if (msg.msg_type == 1) { 
        if (SensitiveFilter::GetInstance().HasSensitiveWord(msg.content)) {
            LOG_WARN("Sensitive word detected in group chat msg: " + msg.msg_id);
            msg.content = SensitiveFilter::GetInstance().Filter(msg.content);
        }
    }
    
    msg.server_time = chat::common::utils::TimeUtils::GetCurrentTimestampMs();
    
    LOG_INFO("Group chat processed, msg_id: " + msg.msg_id);
    return true;
}

} // namespace chat::logic
