#pragma once
#include <string>
#include <vector>
#include "chat_handler.h"

namespace chat::message {

// 抽象索引存储，主要用来存储用户信箱里的索引指针 (seq_id -> msg_id)
class IMessageIndex {
public:
    virtual ~IMessageIndex() = default;

    virtual bool Connect(const std::string& dsn) = 0;

    // 存储消息索引并分配 seq_id
    virtual std::string SaveIndex(const logic::LogicChatMsg& msg) = 0;

    // 拉取大于 begin_seq 的 msg_id 列表 (即离线消息索引)
    virtual std::vector<std::string> FetchOfflineMsgIds(uint64_t user_id, const std::string& begin_seq, int limit) = 0;
};

// MySQL 实现类
class MysqlIndex : public IMessageIndex {
public:
    static MysqlIndex& GetInstance();

    bool Connect(const std::string& dsn) override;
    std::string SaveIndex(const logic::LogicChatMsg& msg) override;
    std::vector<std::string> FetchOfflineMsgIds(uint64_t user_id, const std::string& begin_seq, int limit) override;

private:
    MysqlIndex() = default;
    ~MysqlIndex() = default;

    bool is_connected_ = false;
    // MYSQL* conn_; // 真实环境
};

} // namespace chat::message
