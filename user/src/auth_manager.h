#pragma once
#include <string>
#include <tuple>
#include <cstdint>

namespace chat::user {

class AuthManager {
public:
    static AuthManager& GetInstance();

    // 注册账号
    // 返回: <是否成功, 错误信息, 新分配的用户ID>
    std::tuple<bool, std::string, uint64_t> Register(const std::string& username, const std::string& password, const std::string& nickname);

    // 登录账号
    // 返回: <是否成功, 错误信息, 颁发的Token, 用户基本信息 JSON 字符串>
    std::tuple<bool, std::string, std::string, std::string> Login(const std::string& username, const std::string& password, int client_type);

    // 使用 bcrypt 加密密码
    std::string HashPassword(const std::string& plain_text);
    
    // 使用 bcrypt 校验密码
    bool ValidatePassword(const std::string& plain_text, const std::string& hash_pass);

    // 登出账号
    bool Logout(uint64_t user_id, const std::string& token);

private:
    AuthManager() = default;
    ~AuthManager() = default;

    // ==========================================
    // DB Operations
    // ==========================================
    uint64_t GetUserIdByUsername(const std::string& username);
    bool CheckPassword(uint64_t user_id, const std::string& password);
};

} // namespace chat::user
