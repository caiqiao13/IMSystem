#pragma once
#include <string>
#include <vector>
#include "logic_chat_msg.h" // Ensure logic_chat_msg.h is available or create a MsgIndex struct

namespace chat::message {

struct MsgIndex {
    std::string msg_id;
    uint64_t sender_id;
    uint64_t receiver_id;
    int session_type;
};

class MysqlIndex {
public:
    static MysqlIndex& GetInstance();

    // 存储消息索引并返回自增 seq_id
    std::string InsertIndex(const MsgIndex& index);

    // 拉取用户离线消息的 ID 列表
    std::vector<std::string> FetchOfflineMsgIds(uint64_t user_id, const std::string& last_seq_id);

private:
    MysqlIndex() = default;
    ~MysqlIndex() = default;
};

} // namespace chat::message
