#pragma once
#include <string>
#include <vector>
#include "chat_handler.h"

namespace chat::message {

// 核心服务类：负责将 MongoDB 和 MySQL 拼接起来，实现全量持久化与离线拉取
class MessageSync {
public:
    static MessageSync& GetInstance();

    bool Init();

    // 1. 存储消息接口 (上游 Logic 服务调用)
    std::string SaveMessage(const logic::LogicChatMsg& msg);

    // 2. 拉取离线消息接口 (客户端/网关查询)
    std::vector<logic::LogicChatMsg> PullOfflineMessages(uint64_t user_id, const std::string& begin_seq, int limit);

private:
    MessageSync() = default;
    ~MessageSync() = default;
};

} // namespace chat::message
