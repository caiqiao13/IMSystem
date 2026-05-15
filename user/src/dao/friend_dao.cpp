#include "friend_dao.h"
#include "logger/logger.h"
#include "db/mysql_pool.h"
#include "utils/retry.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

namespace chat::user::dao {

FriendDao& FriendDao::GetInstance() {
    static FriendDao instance;
    return instance;
}

bool FriendDao::IsFriend(uint64_t user_id, uint64_t friend_id) {
    try {
        return chat::common::utils::ExecuteWithRetry<bool>(
            3, 500, "DAO IsFriend", [&]() -> bool {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                    "SELECT COUNT(1) AS cnt FROM user_friends WHERE user_id = ? AND friend_id = ? AND status = 1"
                ));
                pstmt->setUInt64(1, user_id);
                pstmt->setUInt64(2, friend_id);
                std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

                if (res->next()) {
                    return res->getInt("cnt") > 0;
                }
                return false;
            });
    } catch (...) {
        LOG_ERROR("[FriendDao] IsFriend failed.");
        return false;
    }
}

bool FriendDao::AddFriend(uint64_t user_id, uint64_t friend_id, const std::string& verify_msg) {
    try {
        chat::common::utils::ExecuteWithRetryVoid(
            3, 500, "DAO AddFriend", [&]() {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                    "INSERT INTO user_friends (user_id, friend_id, status) VALUES (?, ?, 1) "
                    "ON DUPLICATE KEY UPDATE status = 1"
                ));
                pstmt->setUInt64(1, user_id);
                pstmt->setUInt64(2, friend_id);
                pstmt->executeUpdate();
            });
        return true;
    } catch (...) {
        LOG_ERROR("[FriendDao] AddFriend failed.");
        return false;
    }
}

bool FriendDao::RemoveFriend(uint64_t user_id, uint64_t friend_id) {
    try {
        chat::common::utils::ExecuteWithRetryVoid(
            3, 500, "DAO RemoveFriend", [&]() {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                    "DELETE FROM user_friends WHERE user_id = ? AND friend_id = ?"
                ));
                pstmt->setUInt64(1, user_id);
                pstmt->setUInt64(2, friend_id);
                pstmt->executeUpdate();
            });
        return true;
    } catch (...) {
        LOG_ERROR("[FriendDao] RemoveFriend failed.");
        return false;
    }
}

std::vector<model::UserFriend> FriendDao::GetFriendList(uint64_t user_id, int page, int page_size, int& total_count) {
    std::vector<model::UserFriend> result;
    try {
        chat::common::utils::ExecuteWithRetryVoid(
            3, 500, "DAO GetFriendList", [&]() {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                // 1. 查询总数
                std::unique_ptr<sql::PreparedStatement> count_stmt(conn->prepareStatement(
                    "SELECT COUNT(1) AS cnt FROM user_friends WHERE user_id = ? AND status = 1"
                ));
                count_stmt->setUInt64(1, user_id);
                std::unique_ptr<sql::ResultSet> count_res(count_stmt->executeQuery());
                if (count_res->next()) {
                    total_count = count_res->getInt("cnt");
                }

                // 2. 查询分页数据
                std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                    "SELECT id, user_id, friend_id, status, created_at FROM user_friends "
                    "WHERE user_id = ? AND status = 1 LIMIT ? OFFSET ?"
                ));
                pstmt->setUInt64(1, user_id);
                pstmt->setInt(2, page_size);
                pstmt->setInt(3, (page - 1) * page_size);
                std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

                while (res->next()) {
                    model::UserFriend uf;
                    uf.id = res->getUInt64("id");
                    uf.user_id = res->getUInt64("user_id");
                    uf.friend_id = res->getUInt64("friend_id");
                    uf.status = res->getInt("status");
                    result.push_back(uf);
                }
            });
    } catch (...) {
        LOG_ERROR("[FriendDao] GetFriendList failed.");
    }
    return result;
}

} // namespace chat::user::dao
