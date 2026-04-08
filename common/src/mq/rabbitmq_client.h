#pragma once
#include <string>
#include <functional>
#include <memory>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

namespace chat::common {
using MessageCallback = std::function<void(const std::string&)>;

class RabbitMQClient {
public:
    RabbitMQClient();
    ~RabbitMQClient();
    
    bool Connect(const std::string& host, int port, const std::string& user, const std::string& pass);
    bool Publish(const std::string& exchange, const std::string& routing_key, const std::string& message);
    bool Subscribe(const std::string& queue_name, MessageCallback callback);

private:
    AmqpClient::Channel::ptr_t channel_;
};
} // namespace chat::common
