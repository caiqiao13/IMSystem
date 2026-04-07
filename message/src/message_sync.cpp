#include "message_sync.h"
#include "logger/logger.h"
#include "mongo_storage.h"
#include "mysql_index.h"

namespace chat::message {

MessageSync& MessageSync::GetInstance() {
    static MessageSync instance;
    return instance;
}

bool MessageSync::Init() {
    LOG_INFO("Initializing MessageSync Module...");
    // 真实情况可能从配置文件读取
    bool ok1 = MongoStorage::GetInstance().Connect("mongodb://localhost:27017/chat_db");
    bool ok2 = MysqlIndex::GetInstance().Connect("root:pass@tcp(localhost:3306)/chat_db");
    return ok1 && ok2;
}

std::string MessageSync::SaveMessage(const logic::LogicChatMsg& msg) {
    LOG_INFO("Processing SaveMessage request for MSG_ID: " + msg.msg_id);
    
    // 1. 先将庞大的消息内容体存入 MongoDB
    if (!MongoStorage::GetInstance().SaveMessage(msg)) {
        LOG_ERROR("Failed to save message body to MongoDB");
        return "";
    }

    // 2. 然后将索引 (seq_id -> msg_id) 写入 MySQL，并获得自增 seq_id
    std::string new_seq_id = MysqlIndex::GetInstance().SaveIndex(msg);
    if (new_seq_id.empty()) {
        LOG_ERROR("Failed to generate and save seq_id in MySQL");
        return "";
    }

    LOG_INFO("Message Saved Successfully. Allocated SeqID: " + new_seq_id);
    return new_seq_id;
}

std::vector<logic::LogicChatMsg> MessageSync::PullOfflineMessages(uint64_t user_id, const std::string& begin_seq, int limit) {
    LOG_INFO("Processing PullOfflineMessages request for User: " + std::to_string(user_id) + ", begin_seq: " + begin_seq);

    // 1. 先从 MySQL 拉取用户大于 begin_seq 的离线消息 ID 列表
    std::vector<std::string> msg_ids = MysqlIndex::GetInstance().FetchOfflineMsgIds(user_id, begin_seq, limit);
    if (msg_ids.empty()) {
        LOG_INFO("No offline messages found for user " + std::to_string(user_id));
        return {};
    }

    // 2. 再根据 msg_id 列表，批量去 MongoDB 查出消息正文
    std::vector<logic::LogicChatMsg> messages = MongoStorage::GetInstance().GetMessages(msg_ids);
    
    LOG_INFO("Successfully pulled " + std::to_string(messages.size()) + " offline messages.");
    return messages;
}

} // namespace chat::message
