#pragma once
#include <string>

namespace chat::common {
class RedisClient {
public:
    RedisClient();
    ~RedisClient();
    
    bool Connect(const std::string& host, int port);
    bool Set(const std::string& key, const std::string& value, int expire_seconds = 0);
    std::string Get(const std::string& key);
    bool Delete(const std::string& key);
};
} // namespace chat::common
