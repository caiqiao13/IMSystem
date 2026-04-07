#pragma once
#include <string>
#include <functional>

namespace chat::common {
using MessageCallback = std::function<void(const std::string&)>;

class RabbitMQClient {
public:
    RabbitMQClient();
    ~RabbitMQClient();
    
    bool Connect(const std::string& host, int port, const std::string& user, const std::string& pass);
    bool Publish(const std::string& exchange, const std::string& routing_key, const std::string& message);
    bool Subscribe(const std::string& queue_name, MessageCallback callback);
};
} // namespace chat::common
