#include <iostream>
#include <thread>
#include "admin_server.h"
#include "logger/logger.h"

using namespace chat::admin;

int main() {
    // 1. 初始化日志
    chat::common::Logger::GetInstance().Init("logs/admin.log", "info");
    LOG_INFO("Starting Admin Service...");

    try {
        // 2. 启动 Admin HTTP 服务器 (默认运行在 8088 端口)
        short port = 8088;
        int thread_pool_size = 4;
        
        AdminServer server(port, thread_pool_size);
        server.Start();

        LOG_INFO("Admin Service started successfully.");
        
        // 3. 阻塞主线程，保持服务运行
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

    } catch (std::exception& e) {
        LOG_ERROR("Admin Service Exception: " + std::string(e.what()));
    }

    return 0;
}
