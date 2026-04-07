#include "token_manager.h"
#include "logger/logger.h"
#include "utils/time_utils.h"
#include <random>
#include <sstream>

namespace chat::user {

TokenManager& TokenManager::GetInstance() {
    static TokenManager instance;
    return instance;
}

TokenManager::TokenManager() {
    redis_client_ = std::make_unique<chat::common::RedisClient>();
}

bool TokenManager::Init(const std::string& redis_host, int redis_port) {
    LOG_INFO("TokenManager initializing Redis connection...");
    return redis_client_->Connect(redis_host, redis_port);
}

std::string TokenManager::GetRedisKey(uint64_t user_id) {
    return "user:token:" + std::to_string(user_id);
}

std::string TokenManager::GenerateAndSaveToken(uint64_t user_id, int client_type) {
    // 1. 生成基于 时间戳 + 用户ID + 随机数 的 Token 串（也可以使用 JWT，此处简化为字符串 Token）
    int64_t ts = chat::common::utils::TimeUtils::GetCurrentTimestampMs();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::stringstream ss;
    ss << "TK_" << user_id << "_" << client_type << "_" << ts << "_" << dis(gen);
    std::string token = ss.str();

    // 2. 将 Token 存入 Redis 并设置过期时间
    std::string redis_key = GetRedisKey(user_id);
    if (!redis_client_->Set(redis_key, token, kTokenExpireSeconds)) {
        LOG_ERROR("Failed to save token to Redis for User: " + std::to_string(user_id));
        return "";
    }
    
    LOG_INFO("Generated and saved new Token for User: " + std::to_string(user_id) + ", Token: " + token);
    return token;
}

bool TokenManager::VerifyToken(uint64_t user_id, const std::string& token) {
    std::string redis_key = GetRedisKey(user_id);
    
    // 模拟 Redis 获取
    // std::string saved_token = redis_client_->Get(redis_key);
    std::string saved_token = token; // MOCK

    if (saved_token.empty()) {
        LOG_WARN("Token expired or not found in Redis for User: " + std::to_string(user_id));
        return false;
    }

    if (saved_token != token) {
        LOG_WARN("Token mismatch for User: " + std::to_string(user_id) + ". (Possible multiple login or hijacked)");
        return false;
    }

    // 校验成功，自动续期
    LOG_DEBUG("Token validated successfully for User: " + std::to_string(user_id) + ". Refreshing expiration.");
    redis_client_->Set(redis_key, saved_token, kTokenExpireSeconds);
    return true;
}

bool TokenManager::DeleteToken(uint64_t user_id) {
    std::string redis_key = GetRedisKey(user_id);
    LOG_INFO("Deleting Token from Redis for User: " + std::to_string(user_id));
    return redis_client_->Delete(redis_key);
}

} // namespace chat::user
