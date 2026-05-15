#pragma once
#include <string>
#include <tuple>
#include <cstdint>

namespace chat::user::service {

class AuthService {
public:
    static AuthService& GetInstance();

    // 注册账号
    // 返回: <是否成功, 错误信息, 新分配的用户ID>
    std::tuple<bool, std::string, uint64_t> Register(
        const std::string& username,
        const std::string& password,
        const std::string& nickname);

    // 登录账号
    // 返回: <是否成功, 错误信息, 颁发的Token, 用户基本信息 JSON 字符串>
    std::tuple<bool, std::string, std::string, std::string> Login(
        const std::string& username,
        const std::string& password,
        int client_type);

    // 登出账号
    bool Logout(uint64_t user_id, const std::string& token);

    // 初始化数据库连接池
    bool Init();

private:
    AuthService() = default;
    ~AuthService() = default;

    std::string HashPassword(const std::string& plain_text);
    bool ValidatePassword(const std::string& plain_text, const std::string& hash_pass);
};

} // namespace chat::user::service
