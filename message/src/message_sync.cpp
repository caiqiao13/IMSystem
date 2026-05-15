#include "message_sync.h"
#include "service/message_service.h"

namespace chat::message {

MessageSync& MessageSync::GetInstance() {
    static MessageSync instance;
    return instance;
}

bool MessageSync::Init() {
    return service::MessageService::GetInstance().Init();
}

std::string MessageSync::SaveMessage(const logic::LogicChatMsg& msg) {
    return service::MessageService::GetInstance().SaveMessage(msg);
}

std::vector<logic::LogicChatMsg> MessageSync::PullOfflineMessages(
    uint64_t user_id,
    const std::string& begin_seq,
    int limit) {
    return service::MessageService::GetInstance().PullOfflineMessages(user_id, begin_seq, limit);
}

} // namespace chat::message
