#include "tcp_server.h"
#include <json/json.h>
#include <sstream>
#include "utils/retry.h"

// 假设本地有 gRPC 桩代码 (生产环境使用)
#if __has_include("chat.grpc.pb.h")
#include "chat.grpc.pb.h"
#include <grpcpp/grpcpp.h>
using chat::pb::LogicService;
using chat::pb::SendMsgReq;
using chat::pb::SendMsgResp;
#endif

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
                last_active_time_ = chat::common::utils::TimeUtils::GetCurrentTimestampSec();
                std::string msg(read_buffer_, length);
                
                // 协议解析与错误处理
                ParseAndRouteMessage(msg);
                
                DoRead();
            } else {
                LOG_WARN("TCP Read Error or Connection closed: " + ec.message());
                Stop();
            }
        });
}

void TcpConnection::ParseAndRouteMessage(const std::string& raw_msg) {
    if (raw_msg.find("PING") != std::string::npos) {
        Send("PONG\n");
        return;
    }

    // 尝试按照 JSON 协议解析
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream s(raw_msg);

    if (!Json::parseFromStream(reader, s, &root, &errs)) {
        LOG_WARN("Protocol Parse Error (Invalid JSON): " + errs);
        Send("{\"code\": 400, \"msg\": \"Invalid JSON protocol\"}\n");
        return;
    }

    if (!root.isMember("cmd") || !root.isMember("payload")) {
        Send("{\"code\": 400, \"msg\": \"Missing cmd or payload field\"}\n");
        return;
    }

    std::string cmd = root["cmd"].asString();
    Json::Value payload = root["payload"];

    if (cmd == "send_msg") {
        ForwardToLogicService(payload);
    } else {
        Send("{\"code\": 404, \"msg\": \"Unknown command\"}\n");
    }
}

void TcpConnection::ForwardToLogicService(const Json::Value& payload) {
#if __has_include("chat.grpc.pb.h")
    // 使用带重试机制的 gRPC 调用，容忍网络抖动或 Logic 节点瞬时不可用
    chat::common::utils::ExecuteWithRetryVoid(3, 500, "gRPC SendMessage", [&]() {
        // 构建真实的 gRPC 客户端
        auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
        auto stub = LogicService::NewStub(channel);

        SendMsgReq req;
        req.set_msg_id(payload["msg_id"].asString());
        req.set_sender_id(payload["sender_id"].asUInt64());
        req.set_receiver_id(payload["receiver_id"].asUInt64());
        req.set_session_type(payload["session_type"].asInt());
        req.set_msg_type(payload["msg_type"].asInt());
        req.set_content(payload["content"].asString());

        SendMsgResp resp;
        grpc::ClientContext context;
        // 设置超时时间 3 秒
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(3));

        grpc::Status status = stub->SendMessage(&context, req, &resp);
        if (!status.ok()) {
            throw std::runtime_error("gRPC Error: " + status.error_message());
        }

        // 返回给客户端
        Json::Value result;
        result["code"] = resp.code();
        result["msg"] = resp.msg();
        result["seq_id"] = resp.seq_id();
        
        Json::StreamWriterBuilder writer;
        writer["indentation"] = "";
        Send(Json::writeString(writer, result) + "\n");
    });
#else
    // Sandbox Mock
    LOG_INFO("Mock Forwarding to LogicService: " + payload["msg_id"].asString());
    Send("{\"code\": 0, \"msg\": \"SUCCESS\", \"seq_id\": \"MOCK_SEQ_123\"}\n");
#endif
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

void TcpConnection::CheckHeartbeat() {
    if (is_stopped_) return;

    auto self(shared_from_this());
    timer_.expires_after(std::chrono::seconds(5));
    timer_.async_wait([this, self](asio::error_code ec) {
        if (ec || is_stopped_) return;

        auto now = chat::common::utils::TimeUtils::GetCurrentTimestampSec();
        if (now - last_active_time_ > kHeartbeatTimeoutSec) {
            LOG_WARN("Connection Timeout (No heartbeat for " + std::to_string(kHeartbeatTimeoutSec) + "s). Disconnecting.");
            Stop();
        } else {
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
