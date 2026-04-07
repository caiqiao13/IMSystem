#include <iostream>
#include <cassert>
#include <string>
#include "mongo_storage.h"
#include "mysql_index.h"
#include "message_sync.h"

using namespace chat::message;
using namespace chat::logic;

void TestMessageStorage() {
    std::cout << "[Test] Testing Storage Connectors..." << std::endl;

    assert(MongoStorage::GetInstance().Connect("mongodb://mock"));
    assert(MysqlIndex::GetInstance().Connect("mysql://mock"));

    LogicChatMsg msg;
    msg.msg_id = "test-msg-1";
    msg.sender_id = 1001;
    msg.receiver_id = 2002;
    msg.content = "test";

    assert(MongoStorage::GetInstance().SaveMessage(msg));
    std::string seq_id = MysqlIndex::GetInstance().SaveIndex(msg);
    assert(!seq_id.empty());
    
    std::cout << "[Test] Storage Connectors Passed." << std::endl;
}

void TestMessageSync() {
    std::cout << "[Test] Testing MessageSync..." << std::endl;

    assert(MessageSync::GetInstance().Init());

    auto msgs = MessageSync::GetInstance().PullOfflineMessages(2002, "SEQ_0", 10);
    assert(!msgs.empty());
    assert(msgs[0].msg_id == "MSG-UUID-12345678");

    std::cout << "[Test] MessageSync Passed." << std::endl;
}

int main() {
    std::cout << "Running Message Tests..." << std::endl;
    TestMessageStorage();
    TestMessageSync();
    std::cout << "All Message Tests Passed!" << std::endl;
    return 0;
}
