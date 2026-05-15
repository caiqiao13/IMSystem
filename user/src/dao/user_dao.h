#pragma once
#include <string>
#include <optional>
#include <cstdint>
#include "model/user_model.h"

namespace chat::user::dao {

class UserDao {
public:
    static UserDao& GetInstance();

    std::optional<model::User> FindByUsername(const std::string& username);
    std::optional<model::User> FindById(uint64_t user_id);
    bool Insert(const model::User& user, uint64_t& out_user_id);
    bool Update(const model::User& user);
    bool Delete(uint64_t user_id);

private:
    UserDao() = default;
    ~UserDao() = default;
};

} // namespace chat::user::dao
