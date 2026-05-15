#include "friend_service.h"
#include "dao/friend_dao.h"
#include "dao/user_dao.h"
#include "logger/logger.h"

namespace chat::user::service {

FriendService& FriendService::GetInstance() {
    static FriendService instance;
    return instance;
}

bool FriendService::AddFriend(uint64_t user_id, uint64_t target_user_id, const std::string& verify_msg) {
    LOG_INFO("[FriendService] Processing AddFriend. User: " + std::to_string(user_id)
             + " -> Target: " + std::to_string(target_user_id));

    // 1. 检查不能加自己
    if (user_id == target_user_id) {
        LOG_WARN("[FriendService] Cannot add self as friend.");
        return false;
    }

    // 2. 检查目标用户是否存在
    auto target = dao::UserDao::GetInstance().FindById(target_user_id);
    if (!target.has_value()) {
        LOG_WARN("[FriendService] Target user not found: " + std::to_string(target_user_id));
        return false;
    }

    // 3. 检查是否已经是好友
    if (dao::FriendDao::GetInstance().IsFriend(user_id, target_user_id)) {
        LOG_WARN("[FriendService] Already friends.");
        return false;
    }

    // 4. 写入好友关系
    bool ok = dao::FriendDao::GetInstance().AddFriend(user_id, target_user_id, verify_msg);
    if (!ok) {
        LOG_ERROR("[FriendService] Failed to save friend relationship.");
        return false;
    }

    LOG_INFO("[FriendService] Friend relationship saved successfully.");
    return true;
}

std::vector<FriendInfo> FriendService::GetFriendList(uint64_t user_id, int page, int page_size, int& total_count) {
    LOG_INFO("[FriendService] Processing GetFriendList for User: " + std::to_string(user_id));

    std::vector<FriendInfo> result;
    auto friend_list = dao::FriendDao::GetInstance().GetFriendList(user_id, page, page_size, total_count);

    for (const auto& uf : friend_list) {
        auto friend_opt = dao::UserDao::GetInstance().FindById(uf.friend_id);
        if (friend_opt.has_value()) {
            const auto& u = friend_opt.value();
            result.push_back({u.user_id, u.nickname, u.avatar_url, u.status});
        }
    }

    LOG_INFO("[FriendService] Fetched " + std::to_string(result.size()) + " friends.");
    return result;
}

bool FriendService::IsFriend(uint64_t user_id, uint64_t friend_id) {
    return dao::FriendDao::GetInstance().IsFriend(user_id, friend_id);
}

} // namespace chat::user::service
