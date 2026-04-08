#include "push_manager.h"
#include "logger/logger.h"
#include <json/json.h>

namespace chat::push {

PushManager& PushManager::GetInstance() {
    static PushManager instance;
    return instance;
}

bool PushManager::Init() {
    LOG_INFO("PushManager initializing...");

    apns_client_ = std::make_unique<APNsClient>();
    // TODO: 从配置中读取 APNs 证书路径，此处移除硬编码路径
    apns_client_->Init(std::getenv("APNS_CERT_PATH") ? std::getenv("APNS_CERT_PATH") : "/etc/certs/apns_cert.pem", 
                       std::getenv("APNS_KEY_PATH") ? std::getenv("APNS_KEY_PATH") : "/etc/certs/apns_key.pem", 
                       true);

    fcm_client_ = std::make_unique<FCMClient>();
    // 从环境变量中读取 FCM 密钥，避免硬编码泄漏
    const char* fcm_key = std::getenv("FCM_SERVER_KEY");
    if (!fcm_key) {
        LOG_WARN("FCM_SERVER_KEY environment variable not set. Using fallback MOCK key.");
        fcm_client_->Init("MOCK_KEY_PLEASE_CHANGE");
    } else {
        fcm_client_->Init(fcm_key);
    }

    return true;
}

PushTask PushManager::ParseMessage(const std::string& raw_msg) {
    PushTask task;
    task.receiver_id = 0;

    try {
        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errs;
        std::istringstream s(raw_msg);

        if (!Json::parseFromStream(reader, s, &root, &errs)) {
            LOG_ERROR("Failed to parse MQ message as JSON: " + raw_msg + ", errs: " + errs);
            return task;
        }

        // 校验必填字段
        if (!root.isMember("receiver_id") || !root.isMember("msg_type") || !root.isMember("content")) {
            LOG_ERROR("MQ JSON missing required fields: " + raw_msg);
            return task;
        }

        task.receiver_id = root["receiver_id"].asUInt64();
        // MOCK: 解析出发送者名字或群组名字作为 Title
        task.title = "New Message";
        task.content = root["content"].asString();

    } catch (const std::exception& e) {
        LOG_ERROR("Exception parsing MQ JSON message: " + raw_msg + ", what: " + e.what());
    } catch (...) {
        LOG_ERROR("Unknown exception parsing MQ JSON message: " + raw_msg);
    }
    return task;
}

bool PushManager::FetchDeviceToken(uint64_t user_id, int& out_platform, std::string& out_token) {
    // TODO: 调用 Redis 查询该用户的活跃 Token 与设备类型
    // 例如: "user:device:2002" -> "{platform:1, token: 'ios_token_xxxx'}"
    if (user_id == 2002) {
        out_platform = 1; // 1: iOS
        out_token = "apns_device_token_abcdef123456";
        return true;
    } else if (user_id == 3003) {
        out_platform = 2; // 2: Android
        out_token = "fcm_device_token_zyxwv98765";
        return true;
    }
    return false; // 找不到说明用户不需要推送（可能 PC 在线或者已注销）
}

void PushManager::HandleIncomingPushTask(const std::string& raw_mq_message) {
    LOG_INFO("PushManager received MQ Message: " + raw_mq_message);

    // 1. 解析消息
    PushTask task = ParseMessage(raw_mq_message);
    if (task.receiver_id == 0) {
        LOG_WARN("Drop invalid push task.");
        return;
    }

    // 2. 查路由 (平台和 Token)
    if (!FetchDeviceToken(task.receiver_id, task.platform, task.device_token)) {
        LOG_DEBUG("No push token found for user: " + std::to_string(task.receiver_id) + ". Skipped.");
        return;
    }

    // 3. 平台分发下发推送
    if (task.platform == 1) { // iOS
        LOG_INFO("Routing to APNs for User " + std::to_string(task.receiver_id));
        apns_client_->PushMessage(task.device_token, task.title, task.content, "{\"msg_id\":\"mock_123\"}");
    } else if (task.platform == 2) { // Android
        LOG_INFO("Routing to FCM for User " + std::to_string(task.receiver_id));
        fcm_client_->PushMessage(task.device_token, task.title, task.content, "{\"msg_id\":\"mock_123\"}");
    } else {
        LOG_WARN("Unsupported platform: " + std::to_string(task.platform));
    }
}

} // namespace chat::push
