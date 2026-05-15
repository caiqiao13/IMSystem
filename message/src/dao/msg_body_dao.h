#pragma once
#include <string>
#include <vector>
#include "model/msg_model.h"

namespace chat::message::dao {

class MsgBodyDao {
public:
    static MsgBodyDao& GetInstance();

    bool Insert(const model::MsgBody& msg);
    std::vector<model::MsgBody> BatchGet(const std::vector<std::string>& msg_ids);

private:
    MsgBodyDao() = default;
    ~MsgBodyDao() = default;
};

} // namespace chat::message::dao
