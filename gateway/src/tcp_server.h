#pragma once

#include <asio.hpp>
#include <memory>
#include <string>
#include <iostream>
#include "logger/logger.h"
#include "utils/time_utils.h"

namespace chat::gateway {

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(asio::io_context& io_context, asio::ip::tcp::socket socket);
    ~TcpConnection();

    void Start();
    void Stop();
    
    // 发送消息
    void Send(const std::string& msg);

private:
    void DoRead();
    void DoWrite();
    void CheckHeartbeat();

    asio::ip::tcp::socket socket_;
    asio::steady_timer timer_;
    std::string write_msg_;
    char read_buffer_[1024];

    int64_t last_active_time_;
    bool is_stopped_;
    
    // 心跳超时阈值(秒)
    static constexpr int kHeartbeatTimeoutSec = 30;
};

class TcpServer {
public:
    TcpServer(asio::io_context& io_context, short port);
    void Start();

private:
    void DoAccept();

    asio::io_context& io_context_;
    asio::ip::tcp::acceptor acceptor_;
};

} // namespace chat::gateway
