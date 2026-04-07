#include "message_processor.h"
#include "logger/logger.h"
#include "utils/time_utils.h"

namespace chat::logic {

MessageProcessor& MessageProcessor::GetInstance() {
    static MessageProcessor instance;
    return instance;
}

bool MessageProcessor::ProcessIncomingMessage(LogicChatMsg& msg, std::string& out_seq_id) {
    LOG_INFO("Processing incoming message: " + msg.msg_id);

    // 1. 根据 SessionType 路由到不同 Handler
    auto handler = GetHandler(msg.session_type);
    if (!handler) {
        LOG_ERROR("Unknown session type: " + std::to_string(msg.session_type));
        return false;
    }

    // 2. 校验权限和关系
    if (!handler->Validate(msg)) {
        LOG_WARN("Message validation failed: " + msg.msg_id);
        return false;
    }

    // 3. 执行特定逻辑（敏感词过滤等）
    if (!handler->ProcessSend(msg)) {
        LOG_WARN("Message processing failed: " + msg.msg_id);
        return false;
    }

    // 4. 调用持久化服务（Message Service）
    // 注意：持久化时 Message Service 会使用 Snowflake 或 Redis 颁发全局严格递增序号 seq_id
    if (!CallMessageServiceToSave(msg, out_seq_id)) {
        LOG_ERROR("Failed to save message to Message Service: " + msg.msg_id);
        return false;
    }

    // 5. 分发给 Push Service (经 MQ 或 Redis PubSub 推送至网关 -> 接收方)
    if (!DispatchToPushService(msg)) {
        // 分发失败但不影响主流程返回，只是需要等客户端重连时拉取离线消息
        LOG_WARN("Failed to dispatch message to Push Service: " + msg.msg_id);
    }

    LOG_INFO("Successfully processed message: " + msg.msg_id + ", seq_id: " + out_seq_id);
    return true;
}

std::unique_ptr<BaseChatHandler> MessageProcessor::GetHandler(int session_type) {
    if (session_type == 1) { // SESSION_SINGLE
        return std::make_unique<SingleChatHandler>();
    } else if (session_type == 2) { // SESSION_GROUP
        return std::make_unique<GroupChatHandler>();
    }
    return nullptr;
}

bool MessageProcessor::CallMessageServiceToSave(LogicChatMsg& msg, std::string& out_seq_id) {
    // TODO: 使用 gRPC 客户端桩代码调用 Message 微服务的 SaveMsg RPC
    LOG_DEBUG("Calling MessageService.SaveMsg RPC...");
    
    // 模拟从下游获取 seq_id
    // 实际上通常基于 时间戳 + 数据中心 ID + 机器 ID + 序列号
    int64_t timestamp = chat::common::utils::TimeUtils::GetCurrentTimestampMs();
    out_seq_id = "SEQ_" + std::to_string(timestamp) + "_10001";
    msg.seq_id = out_seq_id;
    
    return true;
}

bool MessageProcessor::DispatchToPushService(const LogicChatMsg& msg) {
    // TODO: 写入 RabbitMQ 或 Redis，推送服务监听
    LOG_DEBUG("Publishing to MQ for PushService: " + msg.msg_id);
    
    // std::string routing_key = "push." + std::to_string(msg.receiver_id);
    // RabbitMQClient::GetInstance().Publish("chat.exchange", routing_key, serialized_msg);
    return true;
}

} // namespace chat::logic
