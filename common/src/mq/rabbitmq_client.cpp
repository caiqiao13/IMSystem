#include "mq/rabbitmq_client.h"
#include "logger/logger.h"

namespace chat::common {

RabbitMQClient::RabbitMQClient() {}
RabbitMQClient::~RabbitMQClient() {}

bool RabbitMQClient::Connect(const std::string& host, int port, const std::string& user, const std::string& pass) {
    LOG_INFO("Connecting to RabbitMQ at " + host + ":" + std::to_string(port));
    // TODO: 使用 rabbitmq-c 库实现连接
    return true;
}

bool RabbitMQClient::Publish(const std::string& exchange, const std::string& routing_key, const std::string& message) {
    LOG_DEBUG("RabbitMQ Publish to exchange: " + exchange + ", routing_key: " + routing_key);
    return true;
}

bool RabbitMQClient::Subscribe(const std::string& queue_name, MessageCallback callback) {
    LOG_INFO("RabbitMQ Subscribe to queue: " + queue_name);
    return true;
}

} // namespace chat::common
