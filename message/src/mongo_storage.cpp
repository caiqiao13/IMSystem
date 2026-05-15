#include "mongo_storage.h"
#include "dao/msg_body_dao.h"
#include "logger/logger.h"

namespace chat::message {

MongoStorage& MongoStorage::GetInstance() {
    static MongoStorage instance;
    return instance;
}

bool MongoStorage::InsertMessage(const MsgBody& msg) {
    model::MsgBody body;
    body.msg_id = msg.msg_id;
    body.content = msg.content;
    body.msg_type = msg.msg_type;
    body.session_type = msg.session_type;
    return dao::MsgBodyDao::GetInstance().Insert(body);
}

std::vector<MsgBody> MongoStorage::GetMessages(const std::vector<std::string>& msg_ids) {
    auto bodies = dao::MsgBodyDao::GetInstance().BatchGet(msg_ids);
    std::vector<MsgBody> result;
    for (const auto& b : bodies) {
        MsgBody msg;
        msg.msg_id = b.msg_id;
        msg.content = b.content;
        msg.msg_type = b.msg_type;
        msg.session_type = b.session_type;
        result.push_back(msg);
    }
    return result;
}

} // namespace chat::message
