#pragma once

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <atomic>
#include <condition_variable>

namespace chat::common {

// MySQL 连接池
class MysqlConnectionPool {
public:
    static MysqlConnectionPool& GetInstance();

    bool Initialize(const std::string& host, int port,
                    const std::string& user, const std::string& password,
                    const std::string& database, int pool_size = 10);

    std::shared_ptr<sql::Connection> AcquireConnection();
    void ReleaseConnection(std::shared_ptr<sql::Connection> conn);

    void Shutdown();

private:
    MysqlConnectionPool() = default;
    ~MysqlConnectionPool();

    std::string host_;
    int port_ = 3306;
    std::string user_;
    std::string password_;
    std::string database_;
    int pool_size_ = 10;

    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::shared_ptr<sql::Connection>> pool_;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> shutdown_{false};

    sql::mysql::MySQL_Driver* driver_ = nullptr;
};

// RAII 连接守卫
class MysqlConnectionGuard {
public:
    explicit MysqlConnectionGuard(MysqlConnectionPool* pool);
    ~MysqlConnectionGuard();

    MysqlConnectionGuard(const MysqlConnectionGuard&) = delete;
    MysqlConnectionGuard& operator=(const MysqlConnectionGuard&) = delete;

    std::shared_ptr<sql::Connection> Get();

private:
    MysqlConnectionPool* pool_ = nullptr;
    std::shared_ptr<sql::Connection> conn_;
};

} // namespace chat::common
