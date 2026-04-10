#pragma once
#include <string>
#include <vector>

namespace chat::message {

struct MsgBody {
    std::string msg_id;
    std::string content;
    int msg_type;
    int session_type;
};

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
