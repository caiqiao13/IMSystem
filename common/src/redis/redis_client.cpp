#include "redis/redis_client.h"
#include "logger/logger.h"

namespace chat::common {

RedisClient::RedisClient() {}
RedisClient::~RedisClient() {}

bool RedisClient::Connect(const std::string& host, int port, const std::string& password, int db) {
    try {
        sw::redis::ConnectionOptions opts;
        opts.host = host;
        opts.port = port;
        opts.password = password;
        opts.db = db;
        
        sw::redis::ConnectionPoolOptions pool_opts;
        pool_opts.size = 10; // 设置连接池大小
        
        redis_ = std::make_unique<sw::redis::Redis>(opts, pool_opts);
        
        auto pong = redis_->ping();
        LOG_INFO("Redis connected successfully to " + host + ":" + std::to_string(port) + ", ping: " + pong);
        return true;
    } catch (const sw::redis::Error& e) {
        LOG_ERROR("Redis connection failed: " + std::string(e.what()));
        return false;
    }
}

bool RedisClient::Set(const std::string& key, const std::string& value, int expire_seconds) {
    try {
        if (expire_seconds > 0) {
            redis_->set(key, value, std::chrono::seconds(expire_seconds));
        } else {
            redis_->set(key, value);
        }
        return true;
    } catch (const sw::redis::Error& e) {
        LOG_ERROR("Redis SET error: " + std::string(e.what()));
        return false;
    }
}

std::string RedisClient::Get(const std::string& key) {
    try {
        auto val = redis_->get(key);
        if (val) {
            return *val;
        }
    } catch (const sw::redis::Error& e) {
        LOG_ERROR("Redis GET error: " + std::string(e.what()));
    }
    return "";
}

bool RedisClient::Delete(const std::string& key) {
    try {
        return redis_->del(key) > 0;
    } catch (const sw::redis::Error& e) {
        LOG_ERROR("Redis DEL error: " + std::string(e.what()));
        return false;
    }
}

} // namespace chat::common
