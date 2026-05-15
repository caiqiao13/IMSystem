#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "model/user_model.h"

namespace chat::user::service {

struct FriendInfo {
    uint64_t user_id;
    std::string nickname;
    std::string avatar_url;
    int status;
};

class FriendService {
public:
    static FriendService& GetInstance();

    // 申请添加好友
    bool AddFriend(uint64_t user_id, uint64_t target_user_id, const std::string& verify_msg);

    // 分页获取好友列表
    std::vector<FriendInfo> GetFriendList(uint64_t user_id, int page, int page_size, int& total_count);

    // 检查是否是好友
    bool IsFriend(uint64_t user_id, uint64_t friend_id);

private:
    FriendService() = default;
    ~FriendService() = default;
};

} // namespace chat::user::service
