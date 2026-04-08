#include <iostream>
#include <cassert>
#include "api_handler.h"

using namespace chat::admin;

void TestAdminLogin() {
    std::cout << "[Test] Testing Admin Login API..." << std::endl;
    
    std::string bad_req = R"({"username": "admin", "password": "wrong"})";
    std::string bad_resp = ApiHandler::GetInstance().HandleRoute("POST", "/api/admin/login", bad_req);
    assert(bad_resp.find("\"code\":401") != std::string::npos);

    std::string good_req = R"({"username": "admin", "password": "admin123"})";
    std::string good_resp = ApiHandler::GetInstance().HandleRoute("POST", "/api/admin/login", good_req);
    assert(good_resp.find("\"code\":0") != std::string::npos);

    std::cout << "[Test] Admin Login API Passed." << std::endl;
}

void TestAdminBanUser() {
    std::cout << "[Test] Testing Admin Ban User API..." << std::endl;
    
    std::string req = R"({"user_id": 2002})";
    std::string resp = ApiHandler::GetInstance().HandleRoute("POST", "/api/admin/users/ban", req);
    assert(resp.find("\"code\":0") != std::string::npos);

    std::cout << "[Test] Admin Ban User API Passed." << std::endl;
}

int main() {
    std::cout << "Running Admin Tests..." << std::endl;
    TestAdminLogin();
    TestAdminBanUser();
    std::cout << "All Admin Tests Passed!" << std::endl;
    return 0;
}
