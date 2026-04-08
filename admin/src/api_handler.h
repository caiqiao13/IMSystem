#pragma once
#include <string>

namespace chat::admin {

class ApiHandler {
public:
    static ApiHandler& GetInstance();

    // 核心路由分发
    std::string HandleRoute(const std::string& method, const std::string& path, const std::string& body);

private:
    ApiHandler() = default;
    ~ApiHandler() = default;

    // 1. 登录验证
    std::string Login(const std::string& body);
    
    // 2. 获取用户列表
    std::string GetUsers();

    // 3. 封禁用户 (拉黑)
    std::string BanUser(const std::string& body);

    // 4. 更新敏感词库 (热更配置)
    std::string UpdateConfig(const std::string& body);
};

} // namespace chat::admin
