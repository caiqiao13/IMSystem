#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace chat::logic {

// Trie 树节点，用于高效敏感词匹配
struct TrieNode {
    std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    bool is_end = false;
    
    // std::unique_ptr 会自动递归释放子节点，无需手动析构
    ~TrieNode() = default;
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
    ~SensitiveFilter() = default;
    
    void AddWord(const std::string& word);

    std::unique_ptr<TrieNode> root_;
};

} // namespace chat::logic
