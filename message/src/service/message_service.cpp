#include "message_service.h"
#include "dao/msg_body_dao.h"
#include "dao/msg_index_dao.h"
#include "logger/logger.h"
#include "db/mysql_pool.h"
#include "db/mongo_pool.h"
#include "config/config_manager.h"

namespace chat::message::service {

MessageService& MessageService::GetInstance() {
    static MessageService instance;
    return instance;
}

bool MessageService::Init() {
    LOG_INFO("[MessageService] Initializing...");

    // 1. 初始化 MySQL 连接池
    std::string mysql_host = chat::common::ConfigManager::GetInstance().GetString("dependencies.mysql.host", "127.0.0.1");
    int mysql_port = chat::common::ConfigManager::GetInstance().GetInt("dependencies.mysql.port", 3306);
    std::string mysql_user = chat::common::ConfigManager::GetInstance().GetString("dependencies.mysql.user", "root");
    std::string mysql_pass = chat::common::ConfigManager::GetInstance().GetString("dependencies.mysql.password", "");
    std::string mysql_db = chat::common::ConfigManager::GetInstance().GetString("dependencies.mysql.database", "chat_db");
    int mysql_pool_size = chat::common::ConfigManager::GetInstance().GetInt("dependencies.mysql.pool_size", 10);

    if (!chat::common::MysqlConnectionPool::GetInstance().Initialize(
            mysql_host, mysql_port, mysql_user, mysql_pass, mysql_db, mysql_pool_size)) {
        LOG_ERROR("[MessageService] Failed to initialize MySQL connection pool.");
        return false;
    }

    // 2. 初始化 MongoDB 连接池
    std::string mongo_uri = chat::common::ConfigManager::GetInstance().GetString("dependencies.mongodb.uri", "mongodb://127.0.0.1:27017");
    int mongo_min_pool = chat::common::ConfigManager::GetInstance().GetInt("dependencies.mongodb.min_pool_size", 5);
    int mongo_max_pool = chat::common::ConfigManager::GetInstance().GetInt("dependencies.mongodb.max_pool_size", 20);

    if (!chat::common::MongoConnectionPool::GetInstance().Initialize(mongo_uri, mongo_min_pool, mongo_max_pool)) {
        LOG_ERROR("[MessageService] Failed to initialize MongoDB connection pool.");
        return false;
    }

    LOG_INFO("[MessageService] Initialized successfully.");
    return true;
}

std::string MessageService::SaveMessage(const logic::LogicChatMsg& msg) {
    LOG_INFO("[MessageService] Saving message. MSG_ID: " + msg.msg_id);

    // 1. 存储消息正文到 MongoDB
    model::MsgBody body;
    body.msg_id = msg.msg_id;
    body.content = msg.content;
    body.msg_type = msg.msg_type;
    body.session_type = msg.session_type;

    if (!dao::MsgBodyDao::GetInstance().Insert(body)) {
        LOG_ERROR("[MessageService] Failed to save message body to MongoDB.");
        return "";
    }

    // 2. 存储索引到 MySQL
    model::MsgIndex index;
    index.msg_id = msg.msg_id;
    index.sender_id = msg.sender_id;
    index.receiver_id = msg.receiver_id;
    index.session_type = msg.session_type;

    std::string new_seq_id = dao::MsgIndexDao::GetInstance().Insert(index);
    if (new_seq_id.empty()) {
        LOG_ERROR("[MessageService] Failed to save message index to MySQL.");
        return "";
    }

    LOG_INFO("[MessageService] Message saved. SeqID: " + new_seq_id);
    return new_seq_id;
}

std::vector<logic::LogicChatMsg> MessageService::PullOfflineMessages(
    uint64_t user_id,
    const std::string& begin_seq,
    int limit) {

    LOG_INFO("[MessageService] Pulling offline messages for User: " + std::to_string(user_id));

    // 1. 从 MySQL 拉取消息 ID 列表
    auto msg_ids = dao::MsgIndexDao::GetInstance().FetchOfflineMsgIds(user_id, begin_seq, limit);
    if (msg_ids.empty()) {
        LOG_INFO("[MessageService] No offline messages found.");
        return {};
    }

    // 2. 从 MongoDB 拉取消息正文
    auto bodies = dao::MsgBodyDao::GetInstance().BatchGet(msg_ids);

    // 3. 组装结果
    std::vector<logic::LogicChatMsg> result;
    for (const auto& body : bodies) {
        logic::LogicChatMsg msg;
        msg.msg_id = body.msg_id;
        msg.content = body.content;
        msg.msg_type = body.msg_type;
        msg.session_type = body.session_type;
        result.push_back(msg);
    }

    LOG_INFO("[MessageService] Pulled " + std::to_string(result.size()) + " offline messages.");
    return result;
}

} // namespace chat::message::service
