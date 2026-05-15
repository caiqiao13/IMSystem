#pragma once
#include <string>
#include <vector>
#include "model/msg_model.h"

namespace chat::message {

// MongoStorage 现在作为 Facade，委托给 MsgBodyDao
// 保持原有接口不变，兼容现有调用方
class MongoStorage {
public:
    static MongoStorage& GetInstance();

    // 存储消息正文
    bool InsertMessage(const MsgBody& msg);

    // 批量拉取消息正文
    std::vector<MsgBody> GetMessages(const std::vector<std::string>& msg_ids);

private:
    MongoStorage() = default;
    ~MongoStorage() = default;
};

} // namespace chat::message
