#include "sensitive_filter.h"
#include "logger/logger.h"

namespace chat::logic {

SensitiveFilter::SensitiveFilter() {
    root_ = std::make_unique<TrieNode>();
}

SensitiveFilter& SensitiveFilter::GetInstance() {
    static SensitiveFilter instance;
    return instance;
}

void SensitiveFilter::Init(const std::vector<std::string>& words) {
    LOG_INFO("Initializing SensitiveFilter with " + std::to_string(words.size()) + " words");
    for (const auto& word : words) {
        AddWord(word);
    }
}

void SensitiveFilter::AddWord(const std::string& word) {
    if (word.empty()) return;
    
    TrieNode* node = root_.get();
    for (char c : word) {
        if (node->children.find(c) == node->children.end()) {
            node->children[c] = std::make_unique<TrieNode>();
        }
        node = node->children[c].get();
    }
    node->is_end = true;
}

bool SensitiveFilter::HasSensitiveWord(const std::string& text) {
    if (text.empty() || root_->children.empty()) return false;
    
    for (size_t i = 0; i < text.length(); ++i) {
        TrieNode* node = root_.get();
        size_t j = i;
        while (j < text.length()) {
            char c = text[j];
            if (node->children.find(c) == node->children.end()) {
                break;
            }
            node = node->children[c].get();
            if (node->is_end) {
                return true;
            }
            j++;
        }
    }
    return false;
}

std::string SensitiveFilter::Filter(const std::string& text) {
    if (text.empty() || root_->children.empty()) return text;
    
    std::string result = text;
    size_t i = 0;
    while (i < text.length()) {
        TrieNode* node = root_.get();
        size_t j = i;
        bool found = false;
        
        while (j < text.length()) {
            char c = text[j];
            if (node->children.find(c) == node->children.end()) {
                break;
            }
            node = node->children[c].get();
            if (node->is_end) {
                found = true;
                // 将匹配到的敏感词替换为 *
                for (size_t k = i; k <= j; ++k) {
                    result[k] = '*';
                }
                i = j; // 跳过已匹配的部分
                break;
            }
            j++;
        }
        i++;
    }
    return result;
}

} // namespace chat::logic
