#include <iostream>
#include <cassert>
#include <string>
#include "apns_client.h"
#include "fcm_client.h"
#include "push_manager.h"

using namespace chat::push;

void TestPushClients() {
    std::cout << "[Test] Testing Push Clients..." << std::endl;

    APNsClient apns;
    assert(apns.Init("cert.pem", "key.pem", true));
    assert(apns.PushMessage("token", "title", "body", "{}"));

    FCMClient fcm;
    assert(fcm.Init("server_key"));
    assert(fcm.PushMessage("token", "title", "body", "{}"));

    std::cout << "[Test] Push Clients Passed." << std::endl;
}

void TestPushManager() {
    std::cout << "[Test] Testing PushManager..." << std::endl;
    assert(PushManager::GetInstance().Init());

    // 测试不合法的消息不应该崩溃
    PushManager::GetInstance().HandleIncomingPushTask("invalid_msg");

    // 测试合法的 iOS 消息 (User 2002 对应的设备在 Mock 中被写死)
    PushManager::GetInstance().HandleIncomingPushTask("2002|1|Test content");

    std::cout << "[Test] PushManager Passed." << std::endl;
}

int main() {
    std::cout << "Running Push Tests..." << std::endl;
    TestPushClients();
    TestPushManager();
    std::cout << "All Push Tests Passed!" << std::endl;
    return 0;
}
