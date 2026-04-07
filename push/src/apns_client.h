#pragma once
#include <string>

namespace chat::push {

class APNsClient {
public:
    APNsClient() = default;
    ~APNsClient() = default;

    bool Init(const std::string& cert_path, const std::string& key_path, bool is_sandbox);
    bool PushMessage(const std::string& device_token, const std::string& title, const std::string& body, const std::string& custom_payload);

private:
    std::string apns_url_;
    // CURL* curl_; // 如果用 libcurl
};

} // namespace chat::push
