#pragma once

#include <functional>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <string>
#include "logger/logger.h"

namespace chat::common::utils {

// 通用带重试机制的函数执行器
// max_retries: 最大重试次数 (0表示不重试，即只执行1次)
// delay_ms: 每次重试间的等待毫秒数
// action_name: 用于打日志的业务操作名称
// func: 要执行的函数，返回类型为 T。如果失败则抛出 std::exception 及其子类。
template <typename T>
T ExecuteWithRetry(int max_retries, int delay_ms, const std::string& action_name, std::function<T()> func) {
    int attempt = 0;
    while (true) {
        try {
            return func();
        } catch (const std::exception& e) {
            if (attempt >= max_retries) {
                LOG_ERROR("[" + action_name + "] failed after " + std::to_string(max_retries) + " retries. Final error: " + e.what());
                throw; // 重试耗尽，向上抛出
            }
            LOG_WARN("[" + action_name + "] attempt " + std::to_string(attempt + 1) + " failed: " + e.what() + ". Retrying in " + std::to_string(delay_ms) + "ms...");
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            attempt++;
        } catch (...) {
            if (attempt >= max_retries) {
                LOG_ERROR("[" + action_name + "] failed after " + std::to_string(max_retries) + " retries with unknown exception.");
                throw;
            }
            LOG_WARN("[" + action_name + "] attempt " + std::to_string(attempt + 1) + " failed with unknown exception. Retrying in " + std::to_string(delay_ms) + "ms...");
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            attempt++;
        }
    }
}

// 针对返回 void 的特化重试执行器
inline void ExecuteWithRetryVoid(int max_retries, int delay_ms, const std::string& action_name, std::function<void()> func) {
    int attempt = 0;
    while (true) {
        try {
            func();
            return;
        } catch (const std::exception& e) {
            if (attempt >= max_retries) {
                LOG_ERROR("[" + action_name + "] failed after " + std::to_string(max_retries) + " retries. Final error: " + e.what());
                throw;
            }
            LOG_WARN("[" + action_name + "] attempt " + std::to_string(attempt + 1) + " failed: " + e.what() + ". Retrying in " + std::to_string(delay_ms) + "ms...");
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            attempt++;
        } catch (...) {
            if (attempt >= max_retries) {
                LOG_ERROR("[" + action_name + "] failed after " + std::to_string(max_retries) + " retries with unknown exception.");
                throw;
            }
            LOG_WARN("[" + action_name + "] attempt " + std::to_string(attempt + 1) + " failed with unknown exception. Retrying in " + std::to_string(delay_ms) + "ms...");
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            attempt++;
        }
    }
}

} // namespace chat::common::utils
