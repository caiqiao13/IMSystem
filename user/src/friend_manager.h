#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace chat::user {

// 模拟 UserInfo 结构
struct UserInfoModel {
    uint64_t user_id;
    std::string nickname;
    std::string avatar_url;
    int32_t status;
};

class FriendManager {
public:
    static FriendManager& GetInstance();

    // 申请添加好友 (暂不考虑复杂的通过同意流程，假设单向关注即可聊天)
    bool AddFriend(uint64_t user_id, uint64_t target_user_id, const std::string& verify_msg);

    // 分页获取好友列表
    std::vector<UserInfoModel> GetFriendList(uint64_t user_id, int page, int page_size, int& total_count);

private:
    FriendManager() = default;
    ~FriendManager() = default;
};

} // namespace chat::user
