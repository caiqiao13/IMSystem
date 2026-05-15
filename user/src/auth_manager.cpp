#include "auth_manager.h"
#include "service/auth_service.h"

namespace chat::user {

AuthManager& AuthManager::GetInstance() {
    static AuthManager instance;
    return instance;
}

std::tuple<bool, std::string, uint64_t> AuthManager::Register(
    const std::string& username,
    const std::string& password,
    const std::string& nickname) {
    return service::AuthService::GetInstance().Register(username, password, nickname);
}

std::tuple<bool, std::string, std::string, std::string> AuthManager::Login(
    const std::string& username,
    const std::string& password,
    int client_type) {
    return service::AuthService::GetInstance().Login(username, password, client_type);
}

bool AuthManager::Logout(uint64_t user_id, const std::string& token) {
    return service::AuthService::GetInstance().Logout(user_id, token);
}

bool AuthManager::Init() {
    return service::AuthService::GetInstance().Init();
}

} // namespace chat::user
