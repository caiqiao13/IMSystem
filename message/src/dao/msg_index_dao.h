#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "model/msg_model.h"

namespace chat::message::dao {

class MsgIndexDao {
public:
    static MsgIndexDao& GetInstance();

    std::string Insert(const model::MsgIndex& index);
    std::vector<std::string> FetchOfflineMsgIds(uint64_t user_id, const std::string& last_seq_id, int limit);

private:
    MsgIndexDao() = default;
    ~MsgIndexDao() = default;
};

} // namespace chat::message::dao
