#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include "redis/redis_client.h"

namespace chat::user {

class TokenManager {
public:
    static TokenManager& GetInstance();

    // 初始化 Redis 连接
    bool Init(const std::string& redis_host, int redis_port);

    // 生成并存储用户 Token (设置过期时间，如 7 天)
    std::string GenerateAndSaveToken(uint64_t user_id, int client_type);

    // 校验 Token 是否合法有效，如果有效刷新过期时间并返回 true
    bool VerifyToken(uint64_t user_id, const std::string& token);

    // 删除 Token (注销登录、顶号)
    bool DeleteToken(uint64_t user_id);

private:
    TokenManager();
    ~TokenManager() = default;

    std::string GetRedisKey(uint64_t user_id);

    std::unique_ptr<chat::common::RedisClient> redis_client_;
    static constexpr int kTokenExpireSeconds = 7 * 24 * 3600; // 7 days
};

} // namespace chat::user
