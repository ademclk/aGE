//*******************************************************************************
// aGE
// Copyright (c) 2026
//
// File: log.cpp
// Module: core
//*******************************************************************************
module;

#include <print>
#include <string_view>

module age.core.log;

namespace age {
void log_message(LogLevel log_level, std::string_view message) {
    const char* tag = "INFO";
    if (log_level == LogLevel::error) {
        tag = "ERROR";
    } else if (log_level == LogLevel::warning) {
        tag = "WARN";
    }
    std::println(stderr, "[age][{}] {}", tag, message);
}
}  // namespace age
