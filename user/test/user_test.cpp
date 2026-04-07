#include <iostream>
#include <cassert>
#include <string>
#include "auth_manager.h"
#include "token_manager.h"
#include "friend_manager.h"

using namespace chat::user;

void TestAuthManager() {
    std::cout << "[Test] Testing AuthManager..." << std::endl;
    
    // MOCK 测试用户名
    auto [success, msg, user_id] = AuthManager::GetInstance().Register("test_user", "123456", "Test");
    // test_user 已存在，应当返回 false
    assert(!success);
    assert(msg == "USER_ALREADY_EXISTS");

    auto [success2, msg2, new_id] = AuthManager::GetInstance().Register("new_user", "password", "Newbie");
    assert(success2);
    assert(new_id == 3003); // 预期的 Mock 返回
    
    // 测试 Bcrypt Hash 生成与校验
    std::string plain = "MySecretPass!@#";
    std::string hash = AuthManager::GetInstance().HashPassword(plain);
    assert(hash.length() == 60); // bcrypt 标准长度
    assert(AuthManager::GetInstance().ValidatePassword(plain, hash));
    assert(!AuthManager::GetInstance().ValidatePassword("wrong_pass", hash));

    std::cout << "[Test] AuthManager Passed." << std::endl;
}

void TestFriendManager() {
    std::cout << "[Test] Testing FriendManager..." << std::endl;

    assert(!FriendManager::GetInstance().AddFriend(1001, 1001, "Hello")); // 不能加自己
    assert(FriendManager::GetInstance().AddFriend(1001, 2002, "Hello"));

    int total = 0;
    auto list = FriendManager::GetInstance().GetFriendList(1001, 1, 20, total);
    assert(total == 2);
    assert(list.size() == 2);

    std::cout << "[Test] FriendManager Passed." << std::endl;
}

int main() {
    std::cout << "Running User Tests..." << std::endl;
    TestAuthManager();
    TestFriendManager();
    std::cout << "All User Tests Passed!" << std::endl;
    return 0;
}
