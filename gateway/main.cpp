#include <iostream>
#include <csignal>
#include <asio.hpp>
#include "logger/logger.h"
#include "config/config_manager.h"
#include "gateway_server.h"

using namespace chat::gateway;

std::unique_ptr<GatewayServer> g_server;

void signal_handler(int signal) {
    if (g_server) {
        LOG_INFO("Received signal " + std::to_string(signal) + ", stopping Gateway...");
        g_server->Stop();
    }
}

int main() {
    // 1) 加载配置
    const std::string config_path = "gateway/config/gateway_config.yaml";
    if (!chat::common::ConfigManager::GetInstance().Load(config_path)) {
        // 兜底初始化日志，至少能看到报错
        chat::common::Logger::GetInstance().Init("logs/gateway.log", "info");
        LOG_ERROR("Failed to load config: " + config_path);
        return -1;
    }

    // 2) 初始化日志（从配置读取）
    const std::string log_file = chat::common::ConfigManager::GetInstance().GetString("log.file", "logs/gateway.log");
    const std::string log_level = chat::common::ConfigManager::GetInstance().GetString("log.level", "info");
    chat::common::Logger::GetInstance().Init(log_file, log_level);
    LOG_INFO("Starting Chat System Gateway...");

    // 注册信号处理
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        // 配置: TCP 端口, HTTP 端口, 线程池数量 (通常按 CPU 核心数配置)
        short tcp_port = static_cast<short>(chat::common::ConfigManager::GetInstance().GetInt("server.tcp_port", 9000));
        short http_port = static_cast<short>(chat::common::ConfigManager::GetInstance().GetInt("server.http_port", 8080));
        std::size_t threads = static_cast<std::size_t>(chat::common::ConfigManager::GetInstance().GetInt("server.thread_pool_size", 0));
        if (threads == 0) {
            threads = std::thread::hardware_concurrency();
            if (threads == 0) threads = 4;
        }

        g_server = std::make_unique<GatewayServer>(tcp_port, http_port, threads);
        
        // Run 将会阻塞主线程并分发任务到 asio 线程池
        g_server->Run();

    } catch (std::exception& e) {
        LOG_ERROR("Exception in main: " + std::string(e.what()));
        return -1;
    }

    LOG_INFO("Gateway exited normally.");
    return 0;
}
