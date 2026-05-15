#pragma once
#include <string>
#include <cstdint>

namespace chat::message::model {

struct MsgBody {
    std::string msg_id;
    std::string content;
    int msg_type = 0;
    int session_type = 0;
};

struct MsgIndex {
    std::string msg_id;
    uint64_t sender_id = 0;
    uint64_t receiver_id = 0;
    int session_type = 0;
    std::string seq_id;
    int64_t created_at = 0;
};

} // namespace chat::message::model
