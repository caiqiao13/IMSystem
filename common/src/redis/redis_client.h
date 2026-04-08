#pragma once
#include <string>
#include <memory>
#include <sw/redis++/redis++.h>

namespace chat::common {
class RedisClient {
public:
    RedisClient();
    ~RedisClient();
    
    bool Connect(const std::string& host, int port, const std::string& password = "", int db = 0);
    bool Set(const std::string& key, const std::string& value, int expire_seconds = 0);
    std::string Get(const std::string& key);
    bool Delete(const std::string& key);

private:
    std::unique_ptr<sw::redis::Redis> redis_;
};
} // namespace chat::common
