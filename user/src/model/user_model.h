#pragma once
#include <string>
#include <cstdint>

namespace chat::user::model {

struct User {
    uint64_t user_id = 0;
    std::string username;
    std::string password_hash;
    std::string nickname;
    std::string avatar_url;
    int status = 0;
    int64_t created_at = 0;
    int64_t updated_at = 0;
};

struct UserFriend {
    uint64_t id = 0;
    uint64_t user_id = 0;
    uint64_t friend_id = 0;
    int status = 1;
    int64_t created_at = 0;
};

} // namespace chat::user::model
