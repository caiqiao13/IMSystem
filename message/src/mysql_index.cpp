#include "mysql_index.h"
#include "logger/logger.h"
#include "db/mysql_pool.h"
#include "utils/retry.h"
#include "utils/time_utils.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

namespace chat::message {

MysqlIndex& MysqlIndex::GetInstance() {
    static MysqlIndex instance;
    return instance;
}

std::string MysqlIndex::InsertIndex(const MsgIndex& index) {
    try {
        return chat::common::utils::ExecuteWithRetry<std::string>(3, 500, "DB Insert Message Index", [&]() {
            chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
            auto conn = guard.Get();
            if (!conn) throw std::runtime_error("Failed to get DB connection");

            // 1. 插入到 msg_index 表
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "INSERT INTO messages_index (msg_id, sender_id, receiver_id, session_type, created_at) VALUES (?, ?, ?, ?, ?)"
            ));
            pstmt->setString(1, index.msg_id);
            pstmt->setUInt64(2, index.sender_id);
            pstmt->setUInt64(3, index.receiver_id);
            pstmt->setInt(4, index.session_type);
            
            int64_t now_ms = chat::common::utils::TimeUtils::GetCurrentTimestampMs();
            pstmt->setUInt64(5, now_ms);
            
            pstmt->executeUpdate();
            
            // 2. 获取 seq_id (假设 messages_index 的自增主键就是 seq_id，转为字符串返回)
            std::unique_ptr<sql::PreparedStatement> id_stmt(conn->prepareStatement("SELECT LAST_INSERT_ID() AS id"));
            std::unique_ptr<sql::ResultSet> res(id_stmt->executeQuery());
            if (res->next()) {
                uint64_t seq = res->getUInt64("id");
                return "SEQ_" + std::to_string(now_ms) + "_" + std::to_string(seq);
            }
            return std::string("");
        });
    } catch (...) {
        LOG_ERROR("Failed to insert message index for msg_id: " + index.msg_id);
        return "";
    }
}

std::vector<std::string> MysqlIndex::FetchOfflineMsgIds(uint64_t user_id, const std::string& last_seq_id) {
    std::vector<std::string> msg_ids;
    try {
        chat::common::utils::ExecuteWithRetryVoid(3, 500, "DB Fetch Offline Msg", [&]() {
            chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
            auto conn = guard.Get();
            if (!conn) throw std::runtime_error("Failed to get DB connection");

            // 简化版：通过比对最后拉取的 seq_id，拉取 receiver_id 为该用户的消息 ID (这里仅为示范，不作严谨的字符串比较)
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "SELECT msg_id FROM messages_index WHERE receiver_id = ? AND id > ? ORDER BY id ASC LIMIT 100"
            ));
            pstmt->setUInt64(1, user_id);
            
            // 简单将 last_seq_id 的最后部分解析出来 (假设格式为 SEQ_xxx_id)
            uint64_t last_id = 0;
            auto pos = last_seq_id.find_last_of('_');
            if (pos != std::string::npos) {
                last_id = std::stoull(last_seq_id.substr(pos + 1));
            }
            pstmt->setUInt64(2, last_id);
            
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            while (res->next()) {
                msg_ids.push_back(res->getString("msg_id"));
            }
        });
    } catch (...) {
        LOG_ERROR("Failed to fetch offline messages for user: " + std::to_string(user_id));
    }
    return msg_ids;
}

} // namespace chat::message
