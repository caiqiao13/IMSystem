#pragma once

#include <asio.hpp>
#include <memory>
#include <string>

namespace chat::gateway {

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
public:
    HttpConnection(asio::io_context& io_context, asio::ip::tcp::socket socket);
    ~HttpConnection() = default;

    void Start();

private:
    void DoRead();
    void DoWrite();
    void HandleRequest(const std::string& request_data);

    asio::ip::tcp::socket socket_;
    char read_buffer_[4096];
    std::string response_data_;
};

class HttpServer {
public:
    HttpServer(asio::io_context& io_context, short port);
    void Start();

private:
    void DoAccept();

    asio::io_context& io_context_;
    asio::ip::tcp::acceptor acceptor_;
};

} // namespace chat::gateway
