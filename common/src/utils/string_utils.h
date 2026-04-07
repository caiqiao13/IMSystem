#pragma once
#include <string>
#include <vector>

namespace chat::common::utils {
class StringUtils {
public:
    // 字符串分割
    static std::vector<std::string> Split(const std::string& str, const std::string& delimiter);
    // 去除两端空白字符
    static std::string Trim(const std::string& str);
};
} // namespace chat::common::utils
