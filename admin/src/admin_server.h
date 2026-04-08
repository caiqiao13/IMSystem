#pragma once
#include <asio.hpp>
#include <memory>
#include <string>

namespace chat::admin {

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
public:
    HttpConnection(asio::io_context& io_context);
    ~HttpConnection() = default;

    asio::ip::tcp::socket& Socket() { return socket_; }
    void Start();

private:
    void DoRead();
    void DoWrite();
    void HandleRequest(const std::string& request_data);

    asio::ip::tcp::socket socket_;
    char read_buffer_[4096];
    std::string response_data_;
};

class AdminServer {
public:
    AdminServer(short port, int thread_pool_size);
    ~AdminServer();

    void Start();
    void Stop();

private:
    void DoAccept();

    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> thread_pool_;
    int thread_pool_size_;
};

} // namespace chat::admin
