#include "tcp_server.h"

namespace chat::gateway {

// =======================================================
// TcpConnection
// =======================================================
TcpConnection::TcpConnection(asio::io_context& io_context, asio::ip::tcp::socket socket)
    : socket_(std::move(socket)), 
      timer_(io_context) {
    last_active_time_ = chat::common::utils::TimeUtils::GetCurrentTimestampSec();
}

TcpConnection::~TcpConnection() {
    Stop();
}

void TcpConnection::Start() {
    LOG_INFO("New TCP Connection from: " + socket_.remote_endpoint().address().to_string());
    last_active_time_ = chat::common::utils::TimeUtils::GetCurrentTimestampSec();
    DoRead();
    CheckHeartbeat();
}

void TcpConnection::Stop() {
    bool expected = false;
    if (is_stopped_.compare_exchange_strong(expected, true)) {
        asio::error_code ec;
        timer_.cancel(ec);
        
        if (socket_.is_open()) {
            LOG_INFO("Closing connection: " + socket_.remote_endpoint(ec).address().to_string());
            socket_.close(ec);
        }
    }
}

void TcpConnection::Send(const std::string& msg) {
    if (is_stopped_) return;
    
    write_msg_ = msg;
    DoWrite();
}

void TcpConnection::DoRead() {
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(read_buffer_, sizeof(read_buffer_)),
        [this, self](asio::error_code ec, std::size_t length) {
            if (!ec) {
                // 更新活跃时间 (心跳检测依赖此值)
                last_active_time_ = chat::common::utils::TimeUtils::GetCurrentTimestampSec();
                
                std::string msg(read_buffer_, length);
                // 模拟简单的 PING/PONG
                if (msg.find("PING") != std::string::npos) {
                    Send("PONG\n");
                } else {
                    LOG_DEBUG("TCP Recv: " + msg);
                    // TODO: 路由给 Logic Server 处理业务逻辑
                    Send("ACK: " + msg);
                }
                DoRead();
            } else {
                LOG_WARN("TCP Read Error or Connection closed: " + ec.message());
                Stop();
            }
        });
}

void TcpConnection::DoWrite() {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(write_msg_),
        [this, self](asio::error_code ec, std::size_t /*length*/) {
            if (ec) {
                LOG_WARN("TCP Write Error: " + ec.message());
                Stop();
            }
        });
}

// 异步心跳超时熔断检测
void TcpConnection::CheckHeartbeat() {
    if (is_stopped_) return;

    auto self(shared_from_this());
    // 每 5 秒检查一次
    timer_.expires_after(std::chrono::seconds(5));
    timer_.async_wait([this, self](asio::error_code ec) {
        if (ec || is_stopped_) return;

        auto now = chat::common::utils::TimeUtils::GetCurrentTimestampSec();
        if (now - last_active_time_ > kHeartbeatTimeoutSec) {
            LOG_WARN("Connection Timeout (No heartbeat for " + std::to_string(kHeartbeatTimeoutSec) + "s). Disconnecting.");
            Stop();
        } else {
            // 继续下一轮检查
            CheckHeartbeat();
        }
    });
}

// =======================================================
// TcpServer
// =======================================================
TcpServer::TcpServer(asio::io_context& io_context, short port)
    : io_context_(io_context),
      acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
}

void TcpServer::Start() {
    LOG_INFO("TCP Server starting on port: " + std::to_string(acceptor_.local_endpoint().port()));
    DoAccept();
}

void TcpServer::DoAccept() {
    acceptor_.async_accept(
        [this](asio::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<TcpConnection>(io_context_, std::move(socket))->Start();
            } else {
                LOG_ERROR("TCP Accept Error: " + ec.message());
            }
            DoAccept();
        });
}

} // namespace chat::gateway
