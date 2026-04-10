#include "auth_manager.h"
#include "logger/logger.h"
#include "token_manager.h"
#include "db/mysql_pool.h"
#include "utils/retry.h"
#include <iostream>
#include <json/json.h>
#include <bcrypt/BCrypt.hpp> // 引入 trusch/libbcrypt
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

namespace chat::user {

AuthManager& AuthManager::GetInstance() {
    static AuthManager instance;
    return instance;
}

std::string AuthManager::HashPassword(const std::string& plain_text) {
    // 使用 bcrypt 加密，设置 cost = 12 (即 2^12 次迭代，安全且性能适中)
    // BCrypt::generateHash 会自动生成 salt 并混合到返回的 hash 字符串中
    return BCrypt::generateHash(plain_text, 12);
}

bool AuthManager::ValidatePassword(const std::string& plain_text, const std::string& hash_pass) {
    // 使用 bcrypt 进行校验，它会自动从 hash_pass 中提取 salt 并做同样的慢速运算对比
    return BCrypt::validatePassword(plain_text, hash_pass);
}

uint64_t AuthManager::GetUserIdByUsername(const std::string& username) {
    try {
        return chat::common::utils::ExecuteWithRetry<uint64_t>(3, 500, "DB Query User", [&]() {
            chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
            auto conn = guard.Get();
            if (!conn) throw std::runtime_error("Failed to get DB connection");

            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("SELECT id FROM users WHERE username = ?"));
            pstmt->setString(1, username);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                return res->getUInt64("id");
            }
            return static_cast<uint64_t>(0); // 未找到
        });
    } catch (...) {
        LOG_ERROR("DB Query User failed after retries.");
        return 0;
    }
}

bool AuthManager::CheckPassword(uint64_t user_id, const std::string& password) {
    try {
        std::string db_hash = chat::common::utils::ExecuteWithRetry<std::string>(3, 500, "DB Check Password", [&]() {
            chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
            auto conn = guard.Get();
            if (!conn) throw std::runtime_error("Failed to get DB connection");

            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("SELECT password_hash FROM users WHERE id = ?"));
            pstmt->setUInt64(1, user_id);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            if (res->next()) {
                return res->getString("password_hash");
            }
            return std::string("");
        });

        if (db_hash.empty()) return false;

        return BCrypt::validatePassword(password, db_hash);
    } catch (...) {
        LOG_ERROR("DB Check Password failed after retries.");
        return false;
    }
}

std::tuple<bool, std::string, uint64_t> AuthManager::Register(const std::string& username, const std::string& password, const std::string& nickname) {
    LOG_INFO("Processing Registration for Username: " + username);

    // 1. 检查是否已经存在
    if (GetUserIdByUsername(username) != 0) {
        LOG_WARN("Username already exists: " + username);
        return {false, "USER_ALREADY_EXISTS", 0};
    }

    // 2. Hash 密码
    std::string hashed_pw = HashPassword(password);

    // 3. 写入 MySQL DB
    uint64_t new_user_id = 0;
    try {
        new_user_id = chat::common::utils::ExecuteWithRetry<uint64_t>(3, 500, "DB Insert User", [&]() {
            chat::common::MysqlConnectionGuard guard(&chat::common::MysqlConnectionPool::GetInstance());
            auto conn = guard.Get();
            if (!conn) throw std::runtime_error("Failed to get DB connection");

            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(
                "INSERT INTO users (username, password_hash, nickname) VALUES (?, ?, ?)"
            ));
            pstmt->setString(1, username);
            pstmt->setString(2, hashed_pw);
            pstmt->setString(3, nickname);
            
            pstmt->executeUpdate();
            
            // 获取刚插入的自增ID (LAST_INSERT_ID)
            std::unique_ptr<sql::PreparedStatement> id_stmt(conn->prepareStatement("SELECT LAST_INSERT_ID() AS id"));
            std::unique_ptr<sql::ResultSet> res(id_stmt->executeQuery());
            if (res->next()) {
                return res->getUInt64("id");
            }
            return static_cast<uint64_t>(0);
        });
    } catch (...) {
        LOG_ERROR("DB Insert User failed after retries.");
        return {false, "SERVER_INTERNAL_ERROR", 0};
    }

    if (new_user_id == 0) {
        return {false, "SERVER_INTERNAL_ERROR", 0};
    }

    LOG_INFO("Saved new user to DB. UserID: " + std::to_string(new_user_id) + ", Nickname: " + nickname);

    return {true, "SUCCESS", new_user_id};
}

std::tuple<bool, std::string, std::string, std::string> AuthManager::Login(const std::string& username, const std::string& password, int client_type) {
    LOG_INFO("Processing Login for Username: " + username + " (Client: " + std::to_string(client_type) + ")");

    // 1. 查询用户 ID
    uint64_t user_id = GetUserIdByUsername(username);
    if (user_id == 0) {
        LOG_WARN("Login failed. User not found: " + username);
        return {false, "USER_NOT_FOUND", "", ""};
    }

    // 2. 校验密码
    if (!CheckPassword(user_id, password)) {
        LOG_WARN("Login failed. Password error for user_id: " + std::to_string(user_id));
        return {false, "PASSWORD_ERROR", "", ""};
    }

    // 3. 密码正确，签发 Token
    std::string new_token = TokenManager::GetInstance().GenerateAndSaveToken(user_id, client_type);
    if (new_token.empty()) {
        return {false, "SERVER_INTERNAL_ERROR", "", ""};
    }

    // 4. 返回 用户信息 和 Token
    Json::Value root;
    root["user_id"] = static_cast<Json::UInt64>(user_id);
    root["nickname"] = "MockNickname";
    root["avatar_url"] = "http://mock/avatar.jpg";
    
    Json::StreamWriterBuilder writer;
    writer["indentation"] = ""; // 压缩输出
    std::string user_info_json = Json::writeString(writer, root);
    
    LOG_INFO("Login successful. Issued Token for User: " + std::to_string(user_id));

    return {true, "SUCCESS", new_token, user_info_json};
}

bool AuthManager::Logout(uint64_t user_id, const std::string& token) {
    LOG_INFO("Processing Logout for UserID: " + std::to_string(user_id));

    // 1. 验证传过来的 Token 是否合法且属于当前登录人
    if (!TokenManager::GetInstance().VerifyToken(user_id, token)) {
        LOG_WARN("Logout failed. Invalid or expired token.");
        return false;
    }

    // 2. 删除 Redis 里的 Token，让当前会话失效
    if (!TokenManager::GetInstance().DeleteToken(user_id)) {
        LOG_ERROR("Failed to delete token from Redis during logout.");
        return false;
    }

    LOG_INFO("Logout successful for UserID: " + std::to_string(user_id));
    return true;
}

} // namespace chat::user
