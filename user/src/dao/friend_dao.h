#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "model/user_model.h"

namespace chat::user::dao {

class FriendDao {
public:
    static FriendDao& GetInstance();

    bool IsFriend(uint64_t user_id, uint64_t friend_id);
    bool AddFriend(uint64_t user_id, uint64_t friend_id, const std::string& verify_msg);
    bool RemoveFriend(uint64_t user_id, uint64_t friend_id);
    std::vector<model::UserFriend> GetFriendList(uint64_t user_id, int page, int page_size, int& total_count);

private:
    FriendDao() = default;
    ~FriendDao() = default;
};

} // namespace chat::user::dao
