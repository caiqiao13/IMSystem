#include "http_server.h"
#include "logger/logger.h"
#include <json/json.h>

namespace chat::gateway {

// =======================================================
// HttpConnection
// =======================================================
HttpConnection::HttpConnection(asio::io_context& io_context, asio::ip::tcp::socket socket)
    : socket_(std::move(socket)) {
}

void HttpConnection::Start() {
    DoRead();
}

void HttpConnection::DoRead() {
    auto self(shared_from_this());
    socket_.async_read_some(asio::buffer(read_buffer_, sizeof(read_buffer_)),
        [this, self](asio::error_code ec, std::size_t length) {
            if (!ec) {
                std::string req(read_buffer_, length);
                HandleRequest(req);
            }
        });
}

void HttpConnection::HandleRequest(const std::string& request_data) {
    LOG_DEBUG("HTTP Request: \n" + request_data.substr(0, request_data.find("\r\n")));

    // 使用 jsoncpp 构造 HTTP 200 OK 响应 JSON
    Json::Value root;
    root["status"] = "ok";
    root["service"] = "gateway";
    
    Json::StreamWriterBuilder writer;
    writer["indentation"] = ""; // 压缩输出
    std::string body = Json::writeString(writer, root);

    response_data_ = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: application/json\r\n"
                     "Content-Length: " + std::to_string(body.length()) + "\r\n"
                     "Connection: close\r\n\r\n" +
                     body;
    
    DoWrite();
}

void HttpConnection::DoWrite() {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(response_data_),
        [this, self](asio::error_code ec, std::size_t /*length*/) {
            asio::error_code ignore_ec;
            socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
        });
}

// =======================================================
// HttpServer
// =======================================================
HttpServer::HttpServer(asio::io_context& io_context, short port)
    : io_context_(io_context),
      acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
}

void HttpServer::Start() {
    LOG_INFO("HTTP Server starting on port: " + std::to_string(acceptor_.local_endpoint().port()));
    DoAccept();
}

void HttpServer::DoAccept() {
    acceptor_.async_accept(
        [this](asio::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<HttpConnection>(io_context_, std::move(socket))->Start();
            } else {
                LOG_ERROR("HTTP Accept Error: " + ec.message());
            }
            DoAccept();
        });
}

} // namespace chat::gateway
