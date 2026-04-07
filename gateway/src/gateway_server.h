#pragma once

#include <asio.hpp>
#include <memory>
#include <vector>
#include <thread>
#include "tcp_server.h"
#include "http_server.h"

namespace chat::gateway {

class GatewayServer {
public:
    GatewayServer(short tcp_port, short http_port, std::size_t thread_pool_size);
    ~GatewayServer();

    void Run();
    void Stop();

private:
    asio::io_context io_context_;
    std::shared_ptr<asio::io_context::work> work_;
    std::vector<std::thread> thread_pool_;

    std::unique_ptr<TcpServer> tcp_server_;
    std::unique_ptr<HttpServer> http_server_;
};

} // namespace chat::gateway
