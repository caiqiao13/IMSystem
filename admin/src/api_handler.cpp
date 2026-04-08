#include "api_handler.h"
#include "logger/logger.h"
#include <json/json.h>
#include <sstream>
#include <vector>

namespace chat::admin {

ApiHandler& ApiHandler::GetInstance() {
    static ApiHandler instance;
    return instance;
}

std::string ApiHandler::HandleRoute(const std::string& method, const std::string& path, const std::string& body) {
    if (method == "POST" && path == "/api/admin/login") {
        return Login(body);
    } else if (method == "GET" && path == "/api/admin/users") {
        return GetUsers();
    } else if (method == "POST" && path == "/api/admin/users/ban") {
        return BanUser(body);
    } else if (method == "POST" && path == "/api/admin/config/words") {
        return UpdateConfig(body);
    }
    return ""; // 交给上层返回 404
}

std::string ApiHandler::Login(const std::string& body) {
    Json::Value req, resp;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream s(body);

    if (Json::parseFromStream(reader, s, &req, &errs)) {
        std::string username = req["username"].asString();
        std::string password = req["password"].asString();

        if (username == "admin" && password == "admin123") {
            resp["code"] = 0;
            resp["msg"] = "SUCCESS";
            resp["data"]["token"] = "ADMIN_TOKEN_999999";
            resp["data"]["username"] = "Super Admin";
            LOG_INFO("Admin login successful.");
        } else {
            resp["code"] = 401;
            resp["msg"] = "Invalid username or password";
            LOG_WARN("Admin login failed for user: " + username);
        }
    } else {
        resp["code"] = 400;
        resp["msg"] = "Invalid JSON format";
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    return Json::writeString(writer, resp);
}

std::string ApiHandler::GetUsers() {
    Json::Value resp;
    resp["code"] = 0;
    resp["msg"] = "SUCCESS";
    
    // MOCK: 模拟从数据库里查出来的用户数据，返回给 pure-admin-thin 前端展示
    Json::Value list(Json::arrayValue);
    
    Json::Value user1;
    user1["id"] = 1001;
    user1["username"] = "test_user";
    user1["nickname"] = "Alice";
    user1["status"] = 1; // 1: 正常
    list.append(user1);

    Json::Value user2;
    user2["id"] = 2002;
    user2["username"] = "bad_guy";
    user2["nickname"] = "Spammer";
    user2["status"] = 2; // 2: 封禁
    list.append(user2);

    resp["data"]["list"] = list;
    resp["data"]["total"] = 2;

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    return Json::writeString(writer, resp);
}

std::string ApiHandler::BanUser(const std::string& body) {
    Json::Value req, resp;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream s(body);

    if (Json::parseFromStream(reader, s, &req, &errs)) {
        uint64_t user_id = req["user_id"].asUInt64();
        // MOCK: 这里应该执行 UPDATE users SET status=2 WHERE user_id=?
        // 并通过 gRPC 踢出该用户的网关连接
        LOG_INFO("Admin banned user_id: " + std::to_string(user_id));
        
        resp["code"] = 0;
        resp["msg"] = "User banned successfully";
    } else {
        resp["code"] = 400;
        resp["msg"] = "Invalid JSON format";
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    return Json::writeString(writer, resp);
}

std::string ApiHandler::UpdateConfig(const std::string& body) {
    Json::Value req, resp;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream s(body);

    if (Json::parseFromStream(reader, s, &req, &errs)) {
        Json::Value words = req["words"];
        std::vector<std::string> new_words;
        for (int i = 0; i < words.size(); ++i) {
            new_words.push_back(words[i].asString());
        }
        
        // MOCK: 这里应该将 new_words 写入 ETCD 或 Redis
        // Logic 服务监听到变更后，会重构内存中的 Trie 树
        LOG_INFO("Admin updated sensitive words config. Word count: " + std::to_string(new_words.size()));
        
        resp["code"] = 0;
        resp["msg"] = "Config updated globally";
    } else {
        resp["code"] = 400;
        resp["msg"] = "Invalid JSON format";
    }

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    return Json::writeString(writer, resp);
}

} // namespace chat::admin