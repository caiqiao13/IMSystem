#include "apns_client.h"
#include "logger/logger.h"
#include <iostream>
#include <json/json.h>

namespace chat::push {

bool APNsClient::Init(const std::string& cert_path, const std::string& key_path, bool is_sandbox) {
    apns_url_ = is_sandbox ? "https://api.sandbox.push.apple.com/3/device/" 
                           : "https://api.push.apple.com/3/device/";
    
    LOG_INFO("APNs Client Initialized. Mode: " + std::string(is_sandbox ? "Sandbox" : "Production") + ", URL: " + apns_url_);
    return true;
}

bool APNsClient::PushMessage(const std::string& device_token, const std::string& title, const std::string& body, const std::string& custom_payload) {
    if (device_token.empty()) return false;

    // 1. 使用 jsoncpp 构建 Apple 推送的 JSON Payload (Aps Dictionary)
    Json::Value root;
    root["aps"]["alert"]["title"] = title;
    root["aps"]["alert"]["body"] = body;
    root["aps"]["badge"] = 1;
    root["aps"]["sound"] = "default";
    
    // 尝试解析 custom_payload
    Json::Value custom_data;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream s(custom_payload);
    if (Json::parseFromStream(reader, s, &custom_data, &errs)) {
        root["custom_data"] = custom_data;
    } else {
        root["custom_data"] = custom_payload;
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  "; // 格式化输出，方便日志查看
    std::string json_payload = Json::writeString(writer, root);

    // 2. 发起 HTTP/2 POST 请求到 APNs
    std::string full_url = apns_url_ + device_token;
    LOG_INFO("[APNs Push] Sending POST to: " + full_url);
    LOG_DEBUG("[APNs Payload] \n" + json_payload);

    // 模拟 HTTP/2 成功返回
    LOG_INFO("[APNs Push] Success. DeviceToken: " + device_token);
    return true;
}

} // namespace chat::push
