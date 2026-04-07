#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace chat::logic {

// Trie 树节点，用于高效敏感词匹配
struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    bool is_end = false;
    
    ~TrieNode() {
        for (auto& pair : children) {
            delete pair.second;
        }
    }
};

class SensitiveFilter {
public:
    static SensitiveFilter& GetInstance();
    
    // 初始化并加载敏感词库
    void Init(const std::vector<std::string>& words);
    
    // 检查并替换敏感词为 *
    std::string Filter(const std::string& text);
    
    // 判断是否包含敏感词
    bool HasSensitiveWord(const std::string& text);

private:
    SensitiveFilter();
    ~SensitiveFilter();
    
    void AddWord(const std::string& word);

    TrieNode* root_;
};

} // namespace chat::logic
