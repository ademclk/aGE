//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: log.cppm
// Module: core
//*******************************************************************************
module;

#include <format>
#include <string_view>
#include <utility>

export module age.core.log;

export namespace age {
enum class LogLevel { info, warning, error };
void log_message(LogLevel log_level, std::string_view message);

template <class... Args>
void log_info(std::format_string<Args...> fmt, Args&&... args) {
    log_message(LogLevel::info, std::format(fmt, std::forward<Args>(args)...));
}

template <class... Args>
void log_warning(std::format_string<Args...> fmt, Args&&... args) {
    log_message(LogLevel::warning, std::format(fmt, std::forward<Args>(args)...));
}

template <class... Args>
void log_error(std::format_string<Args...> fmt, Args&&... args) {
    log_message(LogLevel::error, std::format(fmt, std::forward<Args>(args)...));
}
}  // namespace age
