#pragma once
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

namespace chat::common {
class Logger {
public:
    static Logger& GetInstance();
    
    void Init(const std::string& log_file, const std::string& level);

    std::shared_ptr<spdlog::logger> GetLogger() { return logger_; }

private:
    Logger();
    ~Logger() = default;

    std::shared_ptr<spdlog::logger> logger_;
};
} // namespace chat::common

// 为了兼容原有调用习惯，用宏封装一层单参数字符串
#define LOG_DEBUG(msg) chat::common::Logger::GetInstance().GetLogger()->debug(msg)
#define LOG_INFO(msg)  chat::common::Logger::GetInstance().GetLogger()->info(msg)
#define LOG_WARN(msg)  chat::common::Logger::GetInstance().GetLogger()->warn(msg)
#define LOG_ERROR(msg) chat::common::Logger::GetInstance().GetLogger()->error(msg)
