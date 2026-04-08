#include "mq/rabbitmq_client.h"
#include "logger/logger.h"

namespace chat::common {

RabbitMQClient::RabbitMQClient() {}
RabbitMQClient::~RabbitMQClient() {}

bool RabbitMQClient::Connect(const std::string& host, int port, const std::string& user, const std::string& pass) {
    try {
        // 创建 RabbitMQ Channel，内建 TCP 连接和 Heartbeat
        channel_ = AmqpClient::Channel::Create(host, port, user, pass);
        LOG_INFO("Connected to RabbitMQ at " + host + ":" + std::to_string(port));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("RabbitMQ connection failed: " + std::string(e.what()));
        return false;
    }
}

bool RabbitMQClient::Publish(const std::string& exchange, const std::string& routing_key, const std::string& message) {
    if (!channel_) return false;
    try {
        auto msg = AmqpClient::BasicMessage::Create(message);
        channel_->BasicPublish(exchange, routing_key, msg);
        LOG_DEBUG("RabbitMQ Publish to exchange: " + exchange + ", routing_key: " + routing_key);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("RabbitMQ Publish failed: " + std::string(e.what()));
        return false;
    }
}

bool RabbitMQClient::Subscribe(const std::string& queue_name, MessageCallback callback) {
    if (!channel_) return false;
    try {
        // 参数依次为 queue_name, consumer_tag, no_local, no_ack, exclusive, prefetch_count
        // 最佳实践：设置 prefetch_count 为 100，避免 OOM，并关闭 no_ack (手动 Ack)
        std::string consumer_tag = channel_->BasicConsume(queue_name, "", true, false, false, 100);
        LOG_INFO("RabbitMQ Subscribe to queue: " + queue_name);
        
        while (true) {
            AmqpClient::Envelope::ptr_t envelope;
            // 阻塞等待消息，也可设置超时参数避免死锁
            if (channel_->BasicConsumeMessage(consumer_tag, envelope, -1)) { 
                try {
                    callback(envelope->Message()->Body());
                    // 业务逻辑成功后手动 Ack
                    channel_->BasicAck(envelope);
                } catch (const std::exception& e) {
                    LOG_ERROR("Message callback failed: " + std::string(e.what()));
                    // 如果业务处理失败，可以选择 Reject 并重新入队 (requeue=true)
                    channel_->BasicReject(envelope, true);
                }
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("RabbitMQ Subscribe failed: " + std::string(e.what()));
        return false;
    }
}

} // namespace chat::common
