#include "friend_manager.h"
#include "service/friend_service.h"
#include "logger/logger.h"

namespace chat::user {

FriendManager& FriendManager::GetInstance() {
    static FriendManager instance;
    return instance;
}

bool FriendManager::AddFriend(uint64_t user_id, uint64_t target_user_id, const std::string& verify_msg) {
    return service::FriendService::GetInstance().AddFriend(user_id, target_user_id, verify_msg);
}

std::vector<UserInfoModel> FriendManager::GetFriendList(uint64_t user_id, int page, int page_size, int& total_count) {
    auto friends = service::FriendService::GetInstance().GetFriendList(user_id, page, page_size, total_count);

    std::vector<UserInfoModel> result;
    for (const auto& f : friends) {
        result.push_back({f.user_id, f.nickname, f.avatar_url, f.status});
    }
    return result;
}

} // namespace chat::user
