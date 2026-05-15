#include "user_dao.h"
#include "logger/logger.h"
#include "db/mysql_pool.h"
#include "utils/retry.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

namespace chat::user::dao {

UserDao& UserDao::GetInstance() {
    static UserDao instance;
    return instance;
}

std::optional<model::User> UserDao::FindByUsername(const std::string& username) {
    try {
        return chat::common::utils::ExecuteWithRetry<std::optional<model::User>>(
            3, 500, "DAO FindByUsername", [&]() -> std::optional<model::User> {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(
                    conn->prepareStatement("SELECT user_id, username, password_hash, nickname, avatar_url, status FROM users WHERE username = ?")
                );
                pstmt->setString(1, username);
                std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

                if (res->next()) {
                    model::User user;
                    user.user_id = res->getUInt64("user_id");
                    user.username = res->getString("username");
                    user.password_hash = res->getString("password_hash");
                    user.nickname = res->getString("nickname");
                    user.avatar_url = res->getString("avatar_url");
                    user.status = res->getInt("status");
                    return user;
                }
                return std::nullopt;
            });
    } catch (...) {
        LOG_ERROR("[UserDao] FindByUsername failed for: " + username);
        return std::nullopt;
    }
}

std::optional<model::User> UserDao::FindById(uint64_t user_id) {
    try {
        return chat::common::utils::ExecuteWithRetry<std::optional<model::User>>(
            3, 500, "DAO FindById", [&]() -> std::optional<model::User> {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(
                    conn->prepareStatement("SELECT user_id, username, password_hash, nickname, avatar_url, status FROM users WHERE user_id = ?")
                );
                pstmt->setUInt64(1, user_id);
                std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

                if (res->next()) {
                    model::User user;
                    user.user_id = res->getUInt64("user_id");
                    user.username = res->getString("username");
                    user.password_hash = res->getString("password_hash");
                    user.nickname = res->getString("nickname");
                    user.avatar_url = res->getString("avatar_url");
                    user.status = res->getInt("status");
                    return user;
                }
                return std::nullopt;
            });
    } catch (...) {
        LOG_ERROR("[UserDao] FindById failed for user_id: " + std::to_string(user_id));
        return std::nullopt;
    }
}

bool UserDao::Insert(const model::User& user, uint64_t& out_user_id) {
    try {
        out_user_id = chat::common::utils::ExecuteWithRetry<uint64_t>(
            3, 500, "DAO Insert User", [&]() -> uint64_t {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                    "INSERT INTO users (username, password_hash, nickname, avatar_url) VALUES (?, ?, ?, ?)"
                ));
                pstmt->setString(1, user.username);
                pstmt->setString(2, user.password_hash);
                pstmt->setString(3, user.nickname);
                pstmt->setString(4, user.avatar_url);
                pstmt->executeUpdate();

                std::unique_ptr<sql::PreparedStatement> id_stmt(
                    conn->prepareStatement("SELECT LAST_INSERT_ID() AS id")
                );
                std::unique_ptr<sql::ResultSet> res(id_stmt->executeQuery());
                if (res->next()) {
                    return res->getUInt64("id");
                }
                return static_cast<uint64_t>(0);
            });
        return out_user_id != 0;
    } catch (...) {
        out_user_id = 0;
        LOG_ERROR("[UserDao] Insert failed for username: " + user.username);
        return false;
    }
}

bool UserDao::Update(const model::User& user) {
    try {
        chat::common::utils::ExecuteWithRetryVoid(
            3, 500, "DAO Update User", [&]() {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                    "UPDATE users SET nickname = ?, avatar_url = ?, status = ? WHERE user_id = ?"
                ));
                pstmt->setString(1, user.nickname);
                pstmt->setString(2, user.avatar_url);
                pstmt->setInt(3, user.status);
                pstmt->setUInt64(4, user.user_id);
                pstmt->executeUpdate();
            });
        return true;
    } catch (...) {
        LOG_ERROR("[UserDao] Update failed for user_id: " + std::to_string(user.user_id));
        return false;
    }
}

bool UserDao::Delete(uint64_t user_id) {
    try {
        chat::common::utils::ExecuteWithRetryVoid(
            3, 500, "DAO Delete User", [&]() {
                chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
                auto conn = guard.Get();
                if (!conn) throw std::runtime_error("Failed to get DB connection");

                std::unique_ptr<sql::PreparedStatement> pstmt(
                    conn->prepareStatement("DELETE FROM users WHERE user_id = ?")
                );
                pstmt->setUInt64(1, user_id);
                pstmt->executeUpdate();
            });
        return true;
    } catch (...) {
        LOG_ERROR("[UserDao] Delete failed for user_id: " + std::to_string(user_id));
        return false;
    }
}

} // namespace chat::user::dao
