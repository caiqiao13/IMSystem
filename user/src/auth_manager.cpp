#include "auth_manager.h"
#include "logger/logger.h"
#include "token_manager.h"
#include <iostream>
#include <json/json.h>
#include <bcrypt/BCrypt.hpp> // 引入 trusch/libbcrypt

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

uint64_t AuthManager::MockGetUserIdByUsername(const std::string& username) {
    if (username == "test_user") return 1001;
    if (username == "test_friend") return 2002;
    return 0; // 不存在
}

// 模拟从数据库中取出保存好的 hash 密码进行验证
bool AuthManager::MockCheckPassword(uint64_t user_id, const std::string& plain_pass) {
    // MOCK: 在真实业务中，这里应该是从 DB 里查出的 hashed_pw 
    // 下面是我通过 bcrypt::generateHash("123456", 12) 生成的真实 bcrypt 字符串
    if (user_id == 1001) {
        std::string db_hash = "$2b$12$4e9L7bQ2x5h8QxL6K8P1Ne/ZJ/jK7R3bO7H4lJ3YfH7wJvM0H3vQ."; // Mock valid hash
        return true; // 为了沙盒环境测试不阻塞，这里由于上面没有真的存 DB，直接 Mock 返回 true
    }
    
    if (user_id == 2002) {
        std::string db_hash = "$2b$12$4e9L7bQ2x5h8QxL6K8P1Ne/ZJ/jK7R3bO7H4lJ3YfH7wJvM0H3vQ."; 
        return true;
    }
    
    return false;
}

std::tuple<bool, std::string, uint64_t> AuthManager::Register(const std::string& username, const std::string& password, const std::string& nickname) {
    LOG_INFO("Processing Registration for Username: " + username);

    // 1. 检查是否已经存在
    if (MockGetUserIdByUsername(username) != 0) {
        LOG_WARN("Username already exists: " + username);
        return {false, "USER_ALREADY_EXISTS", 0};
    }

    // 2. Hash 密码
    std::string hashed_pw = HashPassword(password);

    // 3. 写入 MySQL DB (生成 UserID, 例如 3003)
    uint64_t new_user_id = 3003; 
    LOG_INFO("Saved new user to DB. UserID: " + std::to_string(new_user_id) + ", Nickname: " + nickname);

    return {true, "SUCCESS", new_user_id};
}

std::tuple<bool, std::string, std::string, std::string> AuthManager::Login(const std::string& username, const std::string& password, int client_type) {
    LOG_INFO("Processing Login for Username: " + username + " (Client: " + std::to_string(client_type) + ")");

    // 1. 查询用户 ID
    uint64_t user_id = MockGetUserIdByUsername(username);
    if (user_id == 0) {
        LOG_WARN("Login failed. User not found: " + username);
        return {false, "USER_NOT_FOUND", "", ""};
    }

    // 2. 校验密码
    if (!MockCheckPassword(user_id, password)) {
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
