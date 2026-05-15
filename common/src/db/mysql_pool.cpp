#include "mysql_pool.h"
#include "logger/logger.h"
#include <sstream>

namespace chat::common {

MysqlConnectionPool::~MysqlConnectionPool() {
    Shutdown();
}

MysqlConnectionPool& MysqlConnectionPool::GetInstance() {
    static MysqlConnectionPool instance;
    return instance;
}

bool MysqlConnectionPool::Initialize(const std::string& host, int port,
                                     const std::string& user, const std::string& password,
                                     const std::string& database, int pool_size) {
    if (initialized_.exchange(true)) {
        return true; // 已经初始化过
    }

    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    database_ = database;
    pool_size_ = pool_size > 0 ? pool_size : 10;

    try {
        driver_ = sql::mysql::get_mysql_driver_instance();

        std::stringstream url;
        url << "tcp://" << host_ << ":" << port_;

        for (int i = 0; i < pool_size_; ++i) {
            std::shared_ptr<sql::Connection> conn(driver_->connect(url.str(), user_, password_));
            conn->setSchema(database_);
            pool_.push(conn);
        }

        LOG_INFO("[MysqlConnectionPool] Initialized with " + std::to_string(pool_size_) + " connections.");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("[MysqlConnectionPool] Initialize failed: ") + e.what());
        {
            std::lock_guard<std::mutex> lock(mtx_);
            while (!pool_.empty()) {
                pool_.pop();
            }
        }
        initialized_ = false;
        return false;
    }
}

std::shared_ptr<sql::Connection> MysqlConnectionPool::AcquireConnection() {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() { return !pool_.empty() || shutdown_; });

    if (shutdown_) {
        return nullptr;
    }

    auto conn = pool_.front();
    pool_.pop();
    return conn;
}

void MysqlConnectionPool::ReleaseConnection(std::shared_ptr<sql::Connection> conn) {
    if (!conn) return;

    std::lock_guard<std::mutex> lock(mtx_);
    if (shutdown_) {
        return;
    }
    pool_.push(conn);
    cv_.notify_one();
}

void MysqlConnectionPool::Shutdown() {
    if (shutdown_.exchange(true)) {
        return;
    }

    std::lock_guard<std::mutex> lock(mtx_);
    while (!pool_.empty()) {
        pool_.pop();
    }
    cv_.notify_all();
    LOG_INFO("[MysqlConnectionPool] Shutdown.");
}

// ==================== MysqlConnectionGuard ====================

MysqlConnectionGuard::MysqlConnectionGuard(MysqlConnectionPool* pool) : pool_(pool) {
    if (pool_) {
        conn_ = pool_->AcquireConnection();
    }
}

MysqlConnectionGuard::~MysqlConnectionGuard() {
    if (pool_ && conn_) {
        pool_->ReleaseConnection(conn_);
    }
}

std::shared_ptr<sql::Connection> MysqlConnectionGuard::Get() {
    return conn_;
}

} // namespace chat::common
