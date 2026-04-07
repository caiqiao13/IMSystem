#include "mysql_index.h"
#include "logger/logger.h"
#include "utils/time_utils.h"

namespace chat::message {

MysqlIndex& MysqlIndex::GetInstance() {
    static MysqlIndex instance;
    return instance;
}

bool MysqlIndex::Connect(const std::string& dsn) {
    LOG_INFO("Connecting to MySQL via DSN: " + dsn);
    // TODO: conn_ = mysql_init(NULL);
    // mysql_real_connect(conn_, "host", "user", "pass", "chat_db", 3306, NULL, 0);
    is_connected_ = true;
    return true;
}

std::string MysqlIndex::SaveIndex(const logic::LogicChatMsg& msg) {
    if (!is_connected_) {
        LOG_ERROR("MySQL not connected. Cannot save message index.");
        return "";
    }

    // 1. 生成基于雪花算法或时间戳的严格递增 SeqId
    int64_t timestamp = common::utils::TimeUtils::GetCurrentTimestampMs();
    std::string seq_id = "SEQ_" + std::to_string(timestamp) + "_M1";

    // 2. 写入 MySQL (类似写扩散或读扩散模型)
    // 例如写扩散下，向接收者的收件箱插入一条记录
    LOG_DEBUG("Saving message index to MySQL for user " + std::to_string(msg.receiver_id) + ", seq_id: " + seq_id);
    
    // std::string sql = "INSERT INTO user_message_index(user_id, session_type, seq_id, msg_id, created_at) ...";
    // mysql_query(conn_, sql.c_str());

    return seq_id;
}

std::vector<std::string> MysqlIndex::FetchOfflineMsgIds(uint64_t user_id, const std::string& begin_seq, int limit) {
    if (!is_connected_) {
        LOG_ERROR("MySQL not connected. Cannot fetch offline message ids.");
        return {};
    }

    LOG_INFO("Fetching offline index from MySQL for user " + std::to_string(user_id) + ", begin_seq: " + begin_seq + ", limit: " + std::to_string(limit));

    // std::string sql = "SELECT msg_id FROM user_message_index WHERE user_id = ? AND seq_id > ? ORDER BY seq_id ASC LIMIT ?";
    // mysql_query(conn_, sql.c_str());
    // MYSQL_RES* res = mysql_store_result(conn_);
    
    std::vector<std::string> result;
    
    // 模拟从数据库拉取到的 MsgIds
    result.push_back("MSG-UUID-12345678");
    result.push_back("MSG-UUID-87654321");
    
    return result;
}

} // namespace chat::message
