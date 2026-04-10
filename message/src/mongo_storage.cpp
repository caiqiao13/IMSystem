#include "mongo_storage.h"
#include "logger/logger.h"
#include "db/mongo_pool.h"
#include "utils/retry.h"
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

namespace chat::message {

MongoStorage& MongoStorage::GetInstance() {
    static MongoStorage instance;
    return instance;
}

bool MongoStorage::InsertMessage(const MsgBody& msg) {
    try {
        return chat::common::utils::ExecuteWithRetry<bool>(3, 500, "Mongo Insert Msg", [&]() {
            auto conn = chat::common::MongoConnectionPool::GetInstance().GetConnection();
            auto collection = conn->database("chat_db")["messages"];

            auto doc_value = document{}
                << "msg_id" << msg.msg_id
                << "content" << msg.content
                << "msg_type" << msg.msg_type
                << "session_type" << msg.session_type
                << finalize;

            auto result = collection.insert_one(doc_value.view());
            return result && result->inserted_id().type() == bsoncxx::type::k_oid;
        });
    } catch (...) {
        LOG_ERROR("Failed to insert message to Mongo: " + msg.msg_id);
        return false;
    }
}

std::vector<MsgBody> MongoStorage::GetMessages(const std::vector<std::string>& msg_ids) {
    std::vector<MsgBody> result;
    if (msg_ids.empty()) return result;

    try {
        chat::common::utils::ExecuteWithRetryVoid(3, 500, "Mongo Fetch Msgs", [&]() {
            auto conn = chat::common::MongoConnectionPool::GetInstance().GetConnection();
            auto collection = conn->database("chat_db")["messages"];

            bsoncxx::builder::stream::array id_array;
            for (const auto& id : msg_ids) {
                id_array << id;
            }

            auto filter = document{} << "msg_id" << bsoncxx::builder::stream::open_document
                                     << "$in" << id_array << bsoncxx::builder::stream::close_document
                                     << finalize;

            auto cursor = collection.find(filter.view());
            for (auto&& doc : cursor) {
                MsgBody msg;
                msg.msg_id = doc["msg_id"].get_string().value.to_string();
                msg.content = doc["content"].get_string().value.to_string();
                msg.msg_type = doc["msg_type"].get_int32().value;
                msg.session_type = doc["session_type"].get_int32().value;
                result.push_back(msg);
            }
        });
    } catch (...) {
        LOG_ERROR("Failed to fetch messages from Mongo.");
    }
    return result;
}

} // namespace chat::message
