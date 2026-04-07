#pragma once
#include <string>
#include <cstdint>

namespace chat::common::utils {
class TimeUtils {
public:
    // 获取当前时间戳(毫秒)
    static int64_t GetCurrentTimestampMs();
    // 获取当前时间戳(秒)
    static int64_t GetCurrentTimestampSec();
    // 获取格式化的时间字符串: YYYY-MM-DD HH:MM:SS
    static std::string GetCurrentTimeString();
};
} // namespace chat::common::utils
