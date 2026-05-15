#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace chat::user {

// 模拟 UserInfo 结构 (保持向后兼容)
struct UserInfoModel {
    uint64_t user_id;
    std::string nickname;
    std::string avatar_url;
    int32_t status;
};

// FriendManager 现在作为 Facade，委托给 FriendService
// 保持原有接口不变，兼容现有调用方
class FriendManager {
public:
    static FriendManager& GetInstance();

    bool AddFriend(uint64_t user_id, uint64_t target_user_id, const std::string& verify_msg);
    std::vector<UserInfoModel> GetFriendList(uint64_t user_id, int page, int page_size, int& total_count);

private:
    FriendManager() = default;
    ~FriendManager() = default;
};

} // namespace chat::user
