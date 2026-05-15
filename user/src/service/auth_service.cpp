#include "auth_service.h"
#include "dao/user_dao.h"
#include "token_manager.h"
#include "logger/logger.h"
#include "db/mysql_pool.h"
#include "config/config_manager.h"
#include <json/json.h>
#include <bcrypt/BCrypt.hpp>

namespace chat::user::service {

AuthService& AuthService::GetInstance() {
    static AuthService instance;
    return instance;
}

bool AuthService::Init() {
    LOG_INFO("[AuthService] Initializing MySQL connection pool...");

    std::string mysql_host = chat::common::ConfigManager::GetInstance().GetString("dependencies.mysql.host", "127.0.0.1");
    int mysql_port = chat::common::ConfigManager::GetInstance().GetInt("dependencies.mysql.port", 3306);
    std::string mysql_user = chat::common::ConfigManager::GetInstance().GetString("dependencies.mysql.user", "root");
    std::string mysql_pass = chat::common::ConfigManager::GetInstance().GetString("dependencies.mysql.password", "");
    std::string mysql_db = chat::common::ConfigManager::GetInstance().GetString("dependencies.mysql.database", "chat_db");
    int mysql_pool_size = chat::common::ConfigManager::GetInstance().GetInt("dependencies.mysql.pool_size", 10);

    if (!chat::common::MysqlConnectionPool::GetInstance().Initialize(
            mysql_host, mysql_port, mysql_user, mysql_pass, mysql_db, mysql_pool_size)) {
        LOG_ERROR("[AuthService] Failed to initialize MySQL connection pool.");
        return false;
    }

    LOG_INFO("[AuthService] MySQL connection pool initialized.");
    return true;
}

std::string AuthService::HashPassword(const std::string& plain_text) {
    return BCrypt::generateHash(plain_text, 12);
}

bool AuthService::ValidatePassword(const std::string& plain_text, const std::string& hash_pass) {
    return BCrypt::validatePassword(plain_text, hash_pass);
}

std::tuple<bool, std::string, uint64_t> AuthService::Register(
    const std::string& username,
    const std::string& password,
    const std::string& nickname) {

    LOG_INFO("[AuthService] Processing Registration for Username: " + username);

    // 1. 检查用户名是否已存在
    auto existing = dao::UserDao::GetInstance().FindByUsername(username);
    if (existing.has_value()) {
        LOG_WARN("[AuthService] Username already exists: " + username);
        return {false, "USER_ALREADY_EXISTS", 0};
    }

    // 2. Hash 密码
    std::string hashed_pw = HashPassword(password);

    // 3. 构建 User 模型并插入
    model::User new_user;
    new_user.username = username;
    new_user.password_hash = hashed_pw;
    new_user.nickname = nickname;
    new_user.avatar_url = "http://mock/avatar.jpg";

    uint64_t new_user_id = 0;
    if (!dao::UserDao::GetInstance().Insert(new_user, new_user_id) || new_user_id == 0) {
        LOG_ERROR("[AuthService] Failed to insert user to DB.");
        return {false, "SERVER_INTERNAL_ERROR", 0};
    }

    LOG_INFO("[AuthService] Registration successful. UserID: " + std::to_string(new_user_id));
    return {true, "SUCCESS", new_user_id};
}

std::tuple<bool, std::string, std::string, std::string> AuthService::Login(
    const std::string& username,
    const std::string& password,
    int client_type) {

    LOG_INFO("[AuthService] Processing Login for Username: " + username);

    // 1. 查询用户
    auto user_opt = dao::UserDao::GetInstance().FindByUsername(username);
    if (!user_opt.has_value()) {
        LOG_WARN("[AuthService] Login failed. User not found: " + username);
        return {false, "USER_NOT_FOUND", "", ""};
    }

    const auto& user = user_opt.value();

    // 2. 校验密码
    if (!ValidatePassword(password, user.password_hash)) {
        LOG_WARN("[AuthService] Login failed. Password error for user_id: " + std::to_string(user.user_id));
        return {false, "PASSWORD_ERROR", "", ""};
    }

    // 3. 签发 Token
    std::string new_token = TokenManager::GetInstance().GenerateAndSaveToken(user.user_id, client_type);
    if (new_token.empty()) {
        return {false, "SERVER_INTERNAL_ERROR", "", ""};
    }

    // 4. 组装用户信息 JSON
    Json::Value root;
    root["user_id"] = static_cast<Json::UInt64>(user.user_id);
    root["username"] = user.username;
    root["nickname"] = user.nickname;
    root["avatar_url"] = user.avatar_url;
    root["status"] = user.status;

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    std::string user_info_json = Json::writeString(writer, root);

    LOG_INFO("[AuthService] Login successful. User: " + std::to_string(user.user_id));
    return {true, "SUCCESS", new_token, user_info_json};
}

bool AuthService::Logout(uint64_t user_id, const std::string& token) {
    LOG_INFO("[AuthService] Processing Logout for UserID: " + std::to_string(user_id));

    // 1. 验证 Token
    if (!TokenManager::GetInstance().VerifyToken(user_id, token)) {
        LOG_WARN("[AuthService] Logout failed. Invalid token.");
        return false;
    }

    // 2. 删除 Token
    if (!TokenManager::GetInstance().DeleteToken(user_id)) {
        LOG_ERROR("[AuthService] Failed to delete token during logout.");
        return false;
    }

    LOG_INFO("[AuthService] Logout successful for UserID: " + std::to_string(user_id));
    return true;
}

} // namespace chat::user::service
