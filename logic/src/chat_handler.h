#pragma once
#include <string>
#include <memory>
#include "sensitive_filter.h"

namespace chat::logic {

// 模拟的 ChatMsg 数据结构 (代替实际的 Protobuf 类，由于沙盒环境没编出 .pb.h)
struct LogicChatMsg {
    std::string msg_id;
    uint64_t sender_id;
    uint64_t receiver_id;
    int session_type; // 1: 单聊, 2: 群聊
    int msg_type;     // 1: 文本
    std::string content;
    int64_t send_time;
    int64_t server_time;
    std::string seq_id;
};

// 抽象处理接口
class BaseChatHandler {
public:
    virtual ~BaseChatHandler() = default;
    
    // 验证消息合法性 (权限、关系)
    virtual bool Validate(const LogicChatMsg& msg) = 0;
    
    // 处理发送逻辑
    virtual bool ProcessSend(LogicChatMsg& msg) = 0;
};

class SingleChatHandler : public BaseChatHandler {
public:
    bool Validate(const LogicChatMsg& msg) override;
    bool ProcessSend(LogicChatMsg& msg) override;
};

class GroupChatHandler : public BaseChatHandler {
public:
    bool Validate(const LogicChatMsg& msg) override;
    bool ProcessSend(LogicChatMsg& msg) override;
};

} // namespace chat::logic
