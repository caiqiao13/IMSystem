#include <iostream>
#include <thread>
#include <chrono>
#include <functional>

#include "logger/logger.h"
#include "config/config_manager.h"
#include "mq/rabbitmq_client.h"
#include "push_manager.h"

using namespace chat::push;
using namespace chat::common;

// 模拟 Push 微服务的长连接启动
void StartPushService() {
    LOG_INFO("Push Service is running and listening to MQ...");

    // 1. 初始化推送管理器 (加载 APNs / FCM 证书及密钥)
    if (!PushManager::GetInstance().Init()) {
        LOG_ERROR("Failed to initialize PushManager");
        return;
    }

    // 2. 连接 RabbitMQ 并订阅 "chat.push.queue"
    // (由于环境没安装 rabbitmq-c，这里直接通过回调模拟消费)
    RabbitMQClient mq_client;
    const std::string mq_host = chat::common::ConfigManager::GetInstance().GetString("dependencies.rabbitmq.host", "127.0.0.1");
    const int mq_port = chat::common::ConfigManager::GetInstance().GetInt("dependencies.rabbitmq.port", 5672);
    const std::string mq_user = chat::common::ConfigManager::GetInstance().GetString("dependencies.rabbitmq.user", "guest");
    const std::string mq_pass = chat::common::ConfigManager::GetInstance().GetString("dependencies.rabbitmq.pass", "guest");
    const std::string mq_queue = chat::common::ConfigManager::GetInstance().GetString("dependencies.rabbitmq.queue", "chat.push.queue");

    if (mq_client.Connect(mq_host, mq_port, mq_user, mq_pass)) {
        // 定义消费者回调函数
        MessageCallback callback = [](const std::string& msg) {
            LOG_INFO("[RabbitMQ Consumer] Received msg from queue: " + msg);
            // 路由给 PushManager 去请求苹果/谷歌的推送服务器
            PushManager::GetInstance().HandleIncomingPushTask(msg);
        };
        
        // 注册到队列
        mq_client.Subscribe(mq_queue, callback);

        // 3. 模拟 Logic Server 往 MQ 投递了两条消息
        std::thread logic_simulator([&callback]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            // 格式约定: {"receiver_id": 2002, "msg_type": 1, "content": "Hello iOS User!"}
            LOG_INFO("Simulating Logic Server pushing to MQ (User 2002 / iOS)...");
            callback(R"({"receiver_id": 2002, "msg_type": 1, "content": "Hello iOS User!"})");
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            LOG_INFO("Simulating Logic Server pushing to MQ (User 3003 / Android)...");
            callback(R"({"receiver_id": 3003, "msg_type": 1, "content": "Hello Android User!"})");
        });

        // 阻塞当前线程，持续消费 MQ
        while (true) {
            std::this_thread::sleep_for(std::chrono::hours(24));
        }
        
        logic_simulator.join();
    }
}

int main() {
    // 1) 加载配置
    const std::string config_path = "push/config/push_config.yaml";
    if (!chat::common::ConfigManager::GetInstance().Load(config_path)) {
        chat::common::Logger::GetInstance().Init("logs/push.log", "info");
        LOG_ERROR("Failed to load config: " + config_path);
        return -1;
    }

    // 2) 初始化日志
    const std::string log_file = chat::common::ConfigManager::GetInstance().GetString("log.file", "logs/push.log");
    const std::string log_level = chat::common::ConfigManager::GetInstance().GetString("log.level", "info");
    chat::common::Logger::GetInstance().Init(log_file, log_level);
    LOG_INFO("Starting Chat System Push Service...");

    try {
        StartPushService();
    } catch (const std::exception& e) {
        LOG_ERROR("Push Service Exception: " + std::string(e.what()));
        return -1;
    }

    return 0;
}
