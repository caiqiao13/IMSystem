#include "gateway_server.h"
#include "logger/logger.h"

namespace chat::gateway {

GatewayServer::GatewayServer(short tcp_port, short http_port, std::size_t thread_pool_size)
    : work_(std::make_shared<asio::io_context::work>(io_context_)) {
    
    tcp_server_ = std::make_unique<TcpServer>(io_context_, tcp_port);
    http_server_ = std::make_unique<HttpServer>(io_context_, http_port);

    // 初始化线程池
    for (std::size_t i = 0; i < thread_pool_size; ++i) {
        thread_pool_.emplace_back([this]() {
            try {
                io_context_.run();
            } catch (const std::exception& e) {
                LOG_ERROR(std::string("Gateway Thread Exception: ") + e.what());
            }
        });
    }
}

GatewayServer::~GatewayServer() {
    Stop();
    for (auto& t : thread_pool_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void GatewayServer::Run() {
    tcp_server_->Start();
    http_server_->Start();
    
    LOG_INFO("Gateway Server is running...");
    
    // 主线程等待，也可以加入信号处理 gracefully shutdown
    for (auto& t : thread_pool_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void GatewayServer::Stop() {
    io_context_.stop();
    work_.reset();
}

} // namespace chat::gateway
