#pragma once
#include <string>

namespace chat::push {

class FCMClient {
public:
    FCMClient() = default;
    ~FCMClient() = default;

    bool Init(const std::string& server_key);
    bool PushMessage(const std::string& device_token, const std::string& title, const std::string& body, const std::string& custom_payload);

private:
    std::string fcm_url_ = "https://fcm.googleapis.com/fcm/send";
    std::string server_key_;
};

} // namespace chat::push
