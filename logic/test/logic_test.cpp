#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "sensitive_filter.h"
#include "chat_handler.h"

using namespace chat::logic;

void TestSensitiveFilter() {
    std::cout << "[Test] Testing SensitiveFilter..." << std::endl;
    
    std::vector<std::string> bad_words = { "badword", "apple", "ban" };
    SensitiveFilter::GetInstance().Init(bad_words);
    
    std::string text1 = "This is a good message.";
    assert(!SensitiveFilter::GetInstance().HasSensitiveWord(text1));
    assert(SensitiveFilter::GetInstance().Filter(text1) == text1);
    
    std::string text2 = "You are a badword!";
    assert(SensitiveFilter::GetInstance().HasSensitiveWord(text2));
    assert(SensitiveFilter::GetInstance().Filter(text2) == "You are a *******!");
    
    std::string text3 = "appleban";
    assert(SensitiveFilter::GetInstance().HasSensitiveWord(text3));
    assert(SensitiveFilter::GetInstance().Filter(text3) == "********");

    std::cout << "[Test] SensitiveFilter Passed." << std::endl;
}

void TestSingleChatHandler() {
    std::cout << "[Test] Testing SingleChatHandler..." << std::endl;
    SingleChatHandler handler;
    LogicChatMsg msg;
    msg.sender_id = 100;
    msg.receiver_id = 100;
    
    // 发给自己应该失败
    assert(!handler.Validate(msg));
    
    msg.receiver_id = 101;
    assert(handler.Validate(msg));
    
    std::cout << "[Test] SingleChatHandler Passed." << std::endl;
}

int main() {
    std::cout << "Running Logic Tests..." << std::endl;
    TestSensitiveFilter();
    TestSingleChatHandler();
    std::cout << "All Logic Tests Passed!" << std::endl;
    return 0;
}
