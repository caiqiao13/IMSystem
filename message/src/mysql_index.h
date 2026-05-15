#pragma once
#include <string>
#include <vector>
#include "model/msg_model.h"

namespace chat::message {

// MysqlIndex 现在作为 Facade，委托给 MsgIndexDao
// 保持原有接口不变，兼容现有调用方
class MysqlIndex {
public:
    static MysqlIndex& GetInstance();

    // 存储消息索引并返回自增 seq_id
    std::string InsertIndex(const MsgIndex& index);

    // 拉取用户离线消息的 ID 列表
    std::vector<std::string> FetchOfflineMsgIds(uint64_t user_id, const std::string& last_seq_id);

private:
    MysqlIndex() = default;
    ~MysqlIndex() = default;
};

} // namespace chat::message
