#include "utils/time_utils.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace chat::common::utils {

int64_t TimeUtils::GetCurrentTimestampMs() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

int64_t TimeUtils::GetCurrentTimestampSec() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

std::string TimeUtils::GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace chat::common::utils
