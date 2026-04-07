#include "mongo_storage.h"
#include "logger/logger.h"

namespace chat::message {

MongoStorage& MongoStorage::GetInstance() {
    static MongoStorage instance;
    return instance;
}

bool MongoStorage::Connect(const std::string& uri) {
    LOG_INFO("Connecting to MongoDB via URI: " + uri);
    // TODO: mongocxx::uri uri(uri);
    //       client_ = mongocxx::client(uri);
    is_connected_ = true;
    return true;
}

bool MongoStorage::SaveMessage(const logic::LogicChatMsg& msg) {
    if (!is_connected_) {
        LOG_ERROR("MongoDB not connected. Cannot save message: " + msg.msg_id);
        return false;
    }

    LOG_DEBUG("Saving message body to MongoDB: " + msg.msg_id);
    
    // TODO: mongocxx::collection collection = client_["chat_db"]["messages"];
    // bsoncxx::builder::stream::document document{};
    // document << "msg_id" << msg.msg_id
    //          << "content" << msg.content
    //          << "msg_type" << msg.msg_type ...
    // collection.insert_one(document.view());

    return true;
}

std::vector<logic::LogicChatMsg> MongoStorage::GetMessages(const std::vector<std::string>& msg_ids) {
    if (!is_connected_) {
        LOG_ERROR("MongoDB not connected. Cannot get messages.");
        return {};
    }

    LOG_INFO("Fetching " + std::to_string(msg_ids.size()) + " messages from MongoDB.");
    
    // TODO: 使用 mongocxx 构建 $in 批量查询
    // 返回对应的 LogicChatMsg 数组
    std::vector<logic::LogicChatMsg> result;
    
    // Mock 返回
    for (const auto& id : msg_ids) {
        logic::LogicChatMsg msg;
        msg.msg_id = id;
        msg.content = "[MOCKED CONTENT FROM MONGO]";
        result.push_back(msg);
    }
    
    return result;
}

} // namespace chat::message
