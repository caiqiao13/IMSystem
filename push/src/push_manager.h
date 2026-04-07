#pragma once
#include <string>
#include <memory>
#include <cstdint>
#include "apns_client.h"
#include "fcm_client.h"

namespace chat::push {

// 内部路由实体
struct PushTask {
    uint64_t receiver_id;
    int platform; // 1: iOS, 2: Android
    std::string device_token;
    std::string title;
    std::string content;
};

class PushManager {
public:
    static PushManager& GetInstance();

    bool Init();

    // 接收从 RabbitMQ 消费到的原始消息进行路由
    void HandleIncomingPushTask(const std::string& raw_mq_message);

private:
    PushManager() = default;
    ~PushManager() = default;

    // 解析 MQ 数据 -> PushTask
    PushTask ParseMessage(const std::string& raw_msg);

    // 查询用户的设备 Token 和 平台信息 (调用 User/Redis 缓存)
    bool FetchDeviceToken(uint64_t user_id, int& out_platform, std::string& out_token);

    std::unique_ptr<APNsClient> apns_client_;
    std::unique_ptr<FCMClient> fcm_client_;
};

} // namespace chat::push
