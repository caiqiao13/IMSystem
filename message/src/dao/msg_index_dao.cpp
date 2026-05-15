#include "msg_index_dao.h"
#include "logger/logger.h"
#include "db/mysql_pool.h"
#include "utils/retry.h"
#include "utils/time_utils.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

namespace chat::message::dao {

MsgIndexDao& MsgIndexDao::GetInstance() {
    static MsgIndexDao instance;
    return instance;
}

std::string MsgIndexDao::Insert(const model::MsgIndex& index) {
    try {
        return chat::common::utils::ExecuteWithRetry<std::string>(
            3, 500, "DAO Insert Message Index", [&]() -> std::string {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                    "INSERT INTO message_index (msg_id, sender_id, receiver_id, session_type, seq_id, created_at) "
                    "VALUES (?, ?, ?, ?, ?, ?)"
                ));
                pstmt->setString(1, index.msg_id);
                pstmt->setUInt64(2, index.sender_id);
                pstmt->setUInt64(3, index.receiver_id);
                pstmt->setInt(4, index.session_type);
                pstmt->setString(5, index.seq_id);

                int64_t now_ms = chat::common::utils::TimeUtils::GetCurrentTimestampMs();
                pstmt->setUInt64(6, now_ms);
                pstmt->executeUpdate();

                std::unique_ptr<sql::PreparedStatement> id_stmt(
                    conn->prepareStatement("SELECT LAST_INSERT_ID() AS id")
                );
                std::unique_ptr<sql::ResultSet> res(id_stmt->executeQuery());
                if (res->next()) {
                    uint64_t seq = res->getUInt64("id");
                    return "SEQ_" + std::to_string(now_ms) + "_" + std::to_string(seq);
                }
                return std::string("");
            });
    } catch (...) {
        LOG_ERROR("[MsgIndexDao] Insert failed for msg_id: " + index.msg_id);
        return "";
    }
}

std::vector<std::string> MsgIndexDao::FetchOfflineMsgIds(uint64_t user_id, const std::string& last_seq_id, int limit) {
    std::vector<std::string> msg_ids;
    try {
        chat::common::utils::ExecuteWithRetryVoid(
            3, 500, "DAO Fetch Offline Msg", [&]() {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                uint64_t last_id = 0;
                auto pos = last_seq_id.find_last_of('_');
                if (pos != std::string::npos) {
                    last_id = std::stoull(last_seq_id.substr(pos + 1));
                }

                std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                    "SELECT msg_id FROM message_index WHERE receiver_id = ? AND id > ? ORDER BY id ASC LIMIT ?"
                ));
                pstmt->setUInt64(1, user_id);
                pstmt->setUInt64(2, last_id);
                pstmt->setInt(3, limit);

                std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
                while (res->next()) {
                    msg_ids.push_back(res->getString("msg_id"));
                }
            });
    } catch (...) {
        LOG_ERROR("[MsgIndexDao] FetchOfflineMsgIds failed for user: " + std::to_string(user_id));
    }
    return msg_ids;
}

} // namespace chat::message::dao
