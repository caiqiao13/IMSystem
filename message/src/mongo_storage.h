#pragma once
#include <string>
#include <vector>
#include "chat_handler.h" // 复用 logic 中的 LogicChatMsg 结构体

namespace chat::message {

// 抽象存储接口，便于将来替换底层数据库实现
class IMessageStorage {
public:
    virtual ~IMessageStorage() = default;

    // 连接数据库
    virtual bool Connect(const std::string& uri) = 0;

    // 存储消息内容体
    virtual bool SaveMessage(const logic::LogicChatMsg& msg) = 0;

    // 批量查询消息内容
    virtual std::vector<logic::LogicChatMsg> GetMessages(const std::vector<std::string>& msg_ids) = 0;
};

// MongoDB 实现类
class MongoStorage : public IMessageStorage {
public:
    static MongoStorage& GetInstance();

    bool Connect(const std::string& uri) override;
    bool SaveMessage(const logic::LogicChatMsg& msg) override;
    std::vector<logic::LogicChatMsg> GetMessages(const std::vector<std::string>& msg_ids) override;

private:
    MongoStorage() = default;
    ~MongoStorage() = default;

    bool is_connected_ = false;
    // mongocxx::client client_; // 真实环境
};

} // namespace chat::message
