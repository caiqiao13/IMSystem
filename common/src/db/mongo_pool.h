#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <memory>
#include <string>
#include <atomic>

namespace chat::common {

// MongoDB 连接池封装
class MongoConnectionPool {
public:
    static MongoConnectionPool& GetInstance();

    bool Initialize(const std::string& uri_string, int min_pool_size = 5, int max_pool_size = 20);

    // 获取一个连接（通过 mongocxx::pool::entry 的包装）
    mongocxx::pool::entry GetConnection();

    void Shutdown();

private:
    MongoConnectionPool() = default;
    ~MongoConnectionPool();

    std::unique_ptr<mongocxx::instance> instance_;
    std::unique_ptr<mongocxx::pool> pool_;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> shutdown_{false};
};

} // namespace chat::common
