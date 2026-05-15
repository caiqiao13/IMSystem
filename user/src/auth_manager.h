#pragma once
#include <string>
#include <tuple>
#include <cstdint>

namespace chat::user {

// AuthManager 现在作为 Facade，委托给 AuthService
// 保持原有接口不变，兼容现有调用方
class AuthManager {
public:
    static AuthManager& GetInstance();

    std::tuple<bool, std::string, uint64_t> Register(
        const std::string& username,
        const std::string& password,
        const std::string& nickname);

    std::tuple<bool, std::string, std::string, std::string> Login(
        const std::string& username,
        const std::string& password,
        int client_type);

    bool Logout(uint64_t user_id, const std::string& token);

    // 初始化数据库连接池
    bool Init();

private:
    AuthManager() = default;
    ~AuthManager() = default;
};

} // namespace chat::user
