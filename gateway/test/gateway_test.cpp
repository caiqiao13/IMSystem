#include <iostream>
#include <cassert>
#include "gateway_server.h"

using namespace chat::gateway;

void TestGatewayInit() {
    std::cout << "[Test] Testing GatewayServer Initialization..." << std::endl;
    // 初始化 GatewayServer 不应该抛出异常
    GatewayServer server(9001, 8081, 2);
    // 测试停止
    server.Stop();
    std::cout << "[Test] GatewayServer Initialization Passed." << std::endl;
}

int main() {
    std::cout << "Running Gateway Tests..." << std::endl;
    TestGatewayInit();
    std::cout << "All Gateway Tests Passed!" << std::endl;
    return 0;
}
