#include <iostream>
#include <thread>
#include <chrono>
#include <functional>

#include "logger/logger.h"
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
    if (mq_client.Connect("127.0.0.1", 5672, "guest", "guest")) {
        // 定义消费者回调函数
        MessageCallback callback = [](const std::string& msg) {
            LOG_INFO("[RabbitMQ Consumer] Received msg from queue: " + msg);
            // 路由给 PushManager 去请求苹果/谷歌的推送服务器
            PushManager::GetInstance().HandleIncomingPushTask(msg);
        };
        
        // 注册到队列
        mq_client.Subscribe("chat.push.queue", callback);

        // 3. 模拟 Logic Server 往 MQ 投递了两条消息
        std::thread logic_simulator([&callback]() {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            // 格式约定: 接收者ID|消息类型|消息体
            LOG_INFO("Simulating Logic Server pushing to MQ (User 2002 / iOS)...");
            callback("2002|1|Hello iOS User!");
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            LOG_INFO("Simulating Logic Server pushing to MQ (User 3003 / Android)...");
            callback("3003|1|Hello Android User!");
        });

        // 阻塞当前线程，持续消费 MQ
        while (true) {
            std::this_thread::sleep_for(std::chrono::hours(24));
        }
        
        logic_simulator.join();
    }
}

int main() {
    // 初始化日志
    chat::common::Logger::GetInstance().Init("push.log", "info");
    LOG_INFO("Starting Chat System Push Service...");

    try {
        StartPushService();
    } catch (const std::exception& e) {
        LOG_ERROR("Push Service Exception: " + std::string(e.what()));
        return -1;
    }

    return 0;
}
