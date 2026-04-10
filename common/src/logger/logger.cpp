#include "logger/logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <iostream>
#include <filesystem>

namespace chat::common {

Logger::Logger() {
    // 默认提供一个终端日志输出器（防止在 Init 调用前使用 LOG_ 宏导致崩溃）
    logger_ = spdlog::stdout_color_mt("default");
    logger_->set_level(spdlog::level::info);
    spdlog::set_default_logger(logger_);
}

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::Init(const std::string& log_file, const std::string& level) {
    try {
        if (!log_file.empty()) {
            std::filesystem::path p(log_file);
            auto parent = p.parent_path();
            if (!parent.empty()) {
                std::error_code ec;
                std::filesystem::create_directories(parent, ec);
                if (ec) {
                    std::cerr << "Failed to create log directory: " << parent.string()
                              << ", error: " << ec.message() << std::endl;
                }
            }
        }

        // 创建终端输出 Sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        
        // 创建按文件大小轮转的 Sink (最大10MB，保留3个文件)
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, 1024 * 1024 * 10, 3);
        
        // 组合 Sink 创建多目标 Logger
        logger_ = std::make_shared<spdlog::logger>("chat_logger", spdlog::sinks_init_list{console_sink, file_sink});
        
        // 设置日志级别
        spdlog::level::level_enum spd_level = spdlog::level::info;
        if (level == "debug") spd_level = spdlog::level::debug;
        else if (level == "warn") spd_level = spdlog::level::warn;
        else if (level == "error") spd_level = spdlog::level::err;
        
        logger_->set_level(spd_level);
        logger_->flush_on(spdlog::level::warn); // 遇到 warn 及以上级别立刻刷盘
        
        // 覆盖默认的 logger
        spdlog::set_default_logger(logger_);
        
        logger_->info("Spdlog Initialized successfully. File: {}, Level: {}", log_file, level);
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

} // namespace chat::common
