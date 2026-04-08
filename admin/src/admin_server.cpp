#include "admin_server.h"
#include "api_handler.h"
#include "logger/logger.h"
#include <iostream>
#include <string>

namespace chat::admin {

// --- HttpConnection ---

HttpConnection::HttpConnection(asio::io_context& io_context)
    : socket_(io_context) {}

void HttpConnection::Start() {
    DoRead();
}

void HttpConnection::DoRead() {
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(read_buffer_),
        [this, self](std::error_code ec, std::size_t length) {
            if (!ec) {
                std::string request(read_buffer_, length);
                HandleRequest(request);
            }
        });
}

void HttpConnection::HandleRequest(const std::string& request_data) {
    auto pos = request_data.find("\r\n");
    if (pos != std::string::npos) {
        LOG_INFO("Admin API Request: " + request_data.substr(0, pos));
    } else {
        LOG_WARN("Malformed HTTP Request received in Admin Server.");
    }

    // 简单解析 HTTP Method 和 Path
    std::string method, path, body;
    size_t method_end = request_data.find(' ');
    if (method_end != std::string::npos) {
        method = request_data.substr(0, method_end);
        size_t path_end = request_data.find(' ', method_end + 1);
        if (path_end != std::string::npos) {
            path = request_data.substr(method_end + 1, path_end - method_end - 1);
        }
    }

    // 简单提取 Body
    size_t body_pos = request_data.find("\r\n\r\n");
    if (body_pos != std::string::npos) {
        body = request_data.substr(body_pos + 4);
    }

    // 路由分发
    std::string response_body;
    int status_code = 200;

    // 处理跨域 (CORS) 供前端 Vue 调试使用
    if (method == "OPTIONS") {
        status_code = 204;
    } else {
        response_body = ApiHandler::GetInstance().HandleRoute(method, path, body);
        if (response_body.empty()) {
            status_code = 404;
            response_body = "{\"code\": 404, \"msg\": \"Not Found\"}";
        }
    }

    response_data_ = "HTTP/1.1 " + std::to_string(status_code) + " OK\r\n"
                     "Content-Type: application/json; charset=utf-8\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                     "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
                     "Content-Length: " + std::to_string(response_body.length()) + "\r\n"
                     "Connection: close\r\n\r\n" +
                     response_body;
    DoWrite();
}

void HttpConnection::DoWrite() {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(response_data_),
        [this, self](std::error_code ec, std::size_t /*length*/) {
            socket_.close();
        });
}

// --- AdminServer ---

AdminServer::AdminServer(short port, int thread_pool_size)
    : io_context_(),
      acceptor_(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      thread_pool_size_(thread_pool_size) {
    LOG_INFO("Admin Server listening on port " + std::to_string(port));
}

AdminServer::~AdminServer() {
    Stop();
}

void AdminServer::Start() {
    DoAccept();
    
    for (int i = 0; i < thread_pool_size_; ++i) {
        thread_pool_.emplace_back([this]() {
            io_context_.run();
        });
    }
}

void AdminServer::Stop() {
    io_context_.stop();
    for (auto& t : thread_pool_) {
        if (t.joinable()) {
            t.join();
        }
    }
    thread_pool_.clear();
}

void AdminServer::DoAccept() {
    auto connection = std::make_shared<HttpConnection>(io_context_);
    acceptor_.async_accept(connection->Socket(),
        [this, connection](std::error_code ec) {
            if (!ec) {
                connection->Start();
            }
            DoAccept();
        });
}

} // namespace chat::admin
