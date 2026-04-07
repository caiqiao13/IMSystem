#include "friend_manager.h"
#include "logger/logger.h"

namespace chat::user {

FriendManager& FriendManager::GetInstance() {
    static FriendManager instance;
    return instance;
}

bool FriendManager::AddFriend(uint64_t user_id, uint64_t target_user_id, const std::string& verify_msg) {
    LOG_INFO("Processing AddFriend. User: " + std::to_string(user_id) + " -> Target: " + std::to_string(target_user_id) + " [MSG: " + verify_msg + "]");

    // 1. 检查不能加自己
    if (user_id == target_user_id) {
        LOG_WARN("Cannot add self as friend.");
        return false;
    }

    // 2. TODO: 查询 MySQL，检查目标用户是否存在
    // SELECT COUNT(1) FROM users WHERE user_id = target_user_id;

    // 3. TODO: 查询 MySQL，检查是否已经是好友
    // SELECT COUNT(1) FROM user_friends WHERE user_id = ... AND friend_id = ...;

    // 4. TODO: 将关系写入 DB，并可通过 MQ 触发离线系统消息推送给 target_user_id
    // INSERT INTO user_friends(user_id, friend_id, status) VALUES ...

    LOG_INFO("Friend relationship saved successfully.");
    return true;
}

std::vector<UserInfoModel> FriendManager::GetFriendList(uint64_t user_id, int page, int page_size, int& total_count) {
    LOG_INFO("Processing GetFriendList for User: " + std::to_string(user_id) + " (Page: " + std::to_string(page) + ", Size: " + std::to_string(page_size) + ")");

    // TODO: 从 MySQL/Redis 查询分页好友列表
    // SELECT u.user_id, u.nickname, u.avatar_url FROM user_friends f JOIN users u ON f.friend_id = u.user_id WHERE f.user_id = ? LIMIT ? OFFSET ?
    
    // 模拟返回
    total_count = 2; // 总好友数
    std::vector<UserInfoModel> friends;

    if (page == 1) {
        friends.push_back({2001, "Alice", "http://avatar/1.jpg", 1});
        friends.push_back({2002, "Bob", "http://avatar/2.jpg", 0});
    }
    
    LOG_INFO("Fetched " + std::to_string(friends.size()) + " friends from DB.");
    return friends;
}

} // namespace chat::user
