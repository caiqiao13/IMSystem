#include "config/config_manager.h"
#include "logger/logger.h"

// Minimal YAML subset parser:
// - Supports nested maps by indentation (spaces)
// - Supports scalar values: number / true/false / quoted or unquoted strings
// - Supports lists of scalar values using "- item" lines
// - Flattens keys with '.' (e.g. server.tcp_port)

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string_view>

namespace chat::common {

namespace {
static inline std::string Trim(std::string_view s) {
    auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
    size_t start = 0;
    while (start < s.size() && is_space(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && is_space(static_cast<unsigned char>(s[end - 1]))) end--;
    return std::string{s.substr(start, end - start)};
}

static inline bool StartsWith(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

static inline std::string Unquote(std::string v) {
    if (v.size() >= 2) {
        char a = v.front();
        char b = v.back();
        if ((a == '"' && b == '"') || (a == '\'' && b == '\'')) {
            v = v.substr(1, v.size() - 2);
        }
    }
    return v;
}

static inline int CountLeadingSpaces(std::string_view s) {
    int n = 0;
    for (char c : s) {
        if (c == ' ') n++;
        else break;
    }
    return n;
}

static inline std::string StripInlineComment(std::string_view s) {
    // Remove inline comments starting with '#' if preceded by whitespace.
    // This is conservative to avoid breaking values that include '#'.
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '#') {
            if (i == 0) return "";
            if (std::isspace(static_cast<unsigned char>(s[i - 1])) != 0) {
                return std::string{s.substr(0, i)};
            }
        }
    }
    return std::string{s};
}

static inline std::string JoinKeys(const std::vector<std::string>& stack) {
    std::string out;
    for (const auto& k : stack) {
        if (k.empty()) continue;
        if (!out.empty()) out.push_back('.');
        out += k;
    }
    return out;
}
} // namespace

ConfigManager& ConfigManager::GetInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::Load(const std::string& file_path) {
    config_map_.clear();

    std::ifstream in(file_path);
    if (!in.is_open()) {
        LOG_ERROR("Failed to open config file: " + file_path);
        return false;
    }

    LOG_INFO("Loading config from: " + file_path);

    std::vector<std::string> key_stack;
    std::vector<int> indent_stack;
    std::unordered_map<std::string, int> list_counters; // key_prefix -> next index

    std::string line;
    int line_no = 0;
    while (std::getline(in, line)) {
        line_no++;

        std::string raw = StripInlineComment(line);
        std::string_view sv = raw;
        if (Trim(sv).empty()) continue;

        int indent = CountLeadingSpaces(sv);
        std::string content = Trim(sv);
        if (content.empty()) continue;

        // Indent stack maintenance
        while (!indent_stack.empty() && indent < indent_stack.back()) {
            indent_stack.pop_back();
            if (!key_stack.empty()) key_stack.pop_back();
        }

        // List item: "- value"
        if (StartsWith(content, "- ")) {
            std::string value = Trim(content.substr(2));
            value = Unquote(Trim(value));

            std::string prefix = JoinKeys(key_stack);
            if (prefix.empty()) {
                LOG_WARN("Ignoring top-level list item at line " + std::to_string(line_no));
                continue;
            }

            int idx = 0;
            auto it = list_counters.find(prefix);
            if (it == list_counters.end()) {
                list_counters[prefix] = 1;
                idx = 0;
            } else {
                idx = it->second;
                it->second++;
            }
            config_map_[prefix + "." + std::to_string(idx)] = value;
            continue;
        }

        // Key/value: "key:" or "key: value"
        auto pos = content.find(':');
        if (pos == std::string::npos) {
            LOG_WARN("Ignoring invalid YAML line " + std::to_string(line_no) + ": " + content);
            continue;
        }

        std::string key = Trim(std::string_view(content).substr(0, pos));
        std::string rest = Trim(std::string_view(content).substr(pos + 1));

        if (key.empty()) {
            LOG_WARN("Ignoring empty key at line " + std::to_string(line_no));
            continue;
        }

        if (rest.empty()) {
            // Map start
            key_stack.push_back(key);
            indent_stack.push_back(indent);
            continue;
        }

        std::string full_key = JoinKeys(key_stack);
        if (!full_key.empty()) full_key.push_back('.');
        full_key += key;

        rest = Unquote(rest);
        config_map_[full_key] = rest;
    }

    return true;
}

std::string ConfigManager::GetString(const std::string& key, const std::string& default_val) {
    auto it = config_map_.find(key);
    return (it != config_map_.end()) ? it->second : default_val;
}

int ConfigManager::GetInt(const std::string& key, int default_val) {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return default_val;
        }
    }
    return default_val;
}

bool ConfigManager::GetBool(const std::string& key, bool default_val) {
    auto it = config_map_.find(key);
    if (it == config_map_.end()) return default_val;

    std::string v = it->second;
    std::transform(v.begin(), v.end(), v.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (v == "true" || v == "1" || v == "yes" || v == "on") return true;
    if (v == "false" || v == "0" || v == "no" || v == "off") return false;
    return default_val;
}

std::vector<std::string> ConfigManager::GetStringList(const std::string& key_prefix) {
    std::vector<std::pair<int, std::string>> items;
    items.reserve(16);

    // List items are stored as "<prefix>.<index>"
    const std::string prefix = key_prefix + ".";
    for (const auto& kv : config_map_) {
        if (!StartsWith(kv.first, prefix)) continue;
        std::string_view suffix = std::string_view(kv.first).substr(prefix.size());
        if (suffix.empty()) continue;
        // Only accept numeric index (avoid accidentally picking nested maps)
        bool all_digits = std::all_of(suffix.begin(), suffix.end(),
                                      [](unsigned char c) { return std::isdigit(c) != 0; });
        if (!all_digits) continue;

        int idx = 0;
        try {
            idx = std::stoi(std::string{suffix});
        } catch (...) {
            continue;
        }
        items.emplace_back(idx, kv.second);
    }

    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<std::string> out;
    out.reserve(items.size());
    for (auto& it : items) out.push_back(std::move(it.second));
    return out;
}

} // namespace chat::common
