#include "mongo_pool.h"
#include "logger/logger.h"
#include <mongocxx/uri.hpp>
#include <stdexcept>
#include <string>

namespace {

// 避免在日志中输出 URI 中的密码（mongodb://user:secret@host/...）
std::string RedactMongoUriForLog(const std::string& uri) {
    const std::string prefix = "mongodb://";
    if (uri.size() < prefix.size() || uri.compare(0, prefix.size(), prefix) != 0) {
        return "<uri>";
    }
    const size_t at = uri.find('@');
    const size_t auth_start = prefix.size();
    const size_t colon = uri.find(':', auth_start);
    if (at != std::string::npos && colon != std::string::npos && colon < at) {
        std::string out = uri;
        out.replace(colon + 1, at - colon - 1, "***");
        return out;
    }
    return uri;
}

} // namespace

namespace chat::common {

MongoConnectionPool::~MongoConnectionPool() {
    Shutdown();
}

MongoConnectionPool& MongoConnectionPool::GetInstance() {
    static MongoConnectionPool instance;
    return instance;
}

bool MongoConnectionPool::Initialize(const std::string& uri_string, int min_pool_size, int max_pool_size) {
    if (initialized_.exchange(true)) {
        return true;
    }

    try {
        instance_ = std::make_unique<mongocxx::instance>();

        mongocxx::uri uri(uri_string);
        mongocxx::options::client_pool pool_options;
        pool_options.min_pool_size(static_cast<size_t>(min_pool_size));
        pool_options.max_pool_size(static_cast<size_t>(max_pool_size));

        pool_ = std::make_unique<mongocxx::pool>(uri, pool_options);

        LOG_INFO("[MongoConnectionPool] Initialized. URI: " + RedactMongoUriForLog(uri_string));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("[MongoConnectionPool] Initialize failed: ") + e.what());
        initialized_ = false;
        return false;
    }
}

mongocxx::pool::entry MongoConnectionPool::GetConnection() {
    if (!pool_ || shutdown_) {
        throw std::runtime_error("MongoConnectionPool not initialized or already shutdown");
    }
    return pool_->acquire();
}

void MongoConnectionPool::Shutdown() {
    if (shutdown_.exchange(true)) {
        return;
    }

    pool_.reset();
    instance_.reset();
    initialized_ = false;
    LOG_INFO("[MongoConnectionPool] Shutdown.");
}

} // namespace chat::common
