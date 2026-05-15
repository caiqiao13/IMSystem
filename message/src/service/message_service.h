#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "chat_handler.h"

namespace chat::message::service {

class MessageService {
public:
    static MessageService& GetInstance();

    bool Init();

    // 存储消息 (上游 Logic 服务调用)
    std::string SaveMessage(const logic::LogicChatMsg& msg);

    // 拉取离线消息
    std::vector<logic::LogicChatMsg> PullOfflineMessages(
        uint64_t user_id,
        const std::string& begin_seq,
        int limit);

private:
    MessageService() = default;
    ~MessageService() = default;
};

} // namespace chat::message::service
