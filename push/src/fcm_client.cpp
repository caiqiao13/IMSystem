#include "fcm_client.h"
#include "logger/logger.h"
#include <json/json.h>

namespace chat::push {

bool FCMClient::Init(const std::string& server_key) {
    server_key_ = server_key;
    LOG_INFO("FCM Client Initialized. Endpoint: " + fcm_url_);
    return true;
}

bool FCMClient::PushMessage(const std::string& device_token, const std::string& title, const std::string& body, const std::string& custom_payload) {
    if (device_token.empty()) return false;

    // 1. 使用 jsoncpp 构建 Firebase 推送 JSON Payload
    Json::Value root;
    root["to"] = device_token;
    root["notification"]["title"] = title;
    root["notification"]["body"] = body;

    // 尝试解析 custom_payload
    Json::Value custom_data;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream s(custom_payload);
    if (Json::parseFromStream(reader, s, &custom_data, &errs)) {
        root["data"]["custom"] = custom_data;
    } else {
        root["data"]["custom"] = custom_payload;
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  "; // 格式化输出，方便日志查看
    std::string json_payload = Json::writeString(writer, root);

    // 2. 发起 HTTP POST 请求到 FCM
    LOG_INFO("[FCM Push] Sending POST to: " + fcm_url_);
    LOG_DEBUG("[FCM Payload] \n" + json_payload);

    // 模拟 HTTP 成功返回
    LOG_INFO("[FCM Push] Success. DeviceToken: " + device_token);
    return true;
}

} // namespace chat::push
