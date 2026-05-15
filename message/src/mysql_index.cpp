#include "mysql_index.h"
#include "dao/msg_index_dao.h"
#include "logger/logger.h"

namespace chat::message {

MysqlIndex& MysqlIndex::GetInstance() {
    static MysqlIndex instance;
    return instance;
}

std::string MysqlIndex::InsertIndex(const MsgIndex& index) {
    model::MsgIndex idx;
    idx.msg_id = index.msg_id;
    idx.sender_id = index.sender_id;
    idx.receiver_id = index.receiver_id;
    idx.session_type = index.session_type;
    return dao::MsgIndexDao::GetInstance().Insert(idx);
}

std::vector<std::string> MysqlIndex::FetchOfflineMsgIds(uint64_t user_id, const std::string& last_seq_id) {
    return dao::MsgIndexDao::GetInstance().FetchOfflineMsgIds(user_id, last_seq_id, 100);
}

} // namespace chat::message
