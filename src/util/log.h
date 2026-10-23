//
// Created by dawid on 10/23/26.
//

#ifndef BSRENDERER_LOG_H
#define BSRENDERER_LOG_H
#include <format>
#include <memory>
#include "defines.h"
#include "platform/platform.h"

class log {
public:
    static void init();

    template<class... Args>
    static void info(std::format_string<Args...> fmt, Args&&... args) {
        const std::string msg = std::format(fmt, std::forward<Args>(args)...);
        const std::string full = "[INFO]: " + msg + "\n";
        platform::log_output(full.c_str(), false, ForegroundConsoleColor::GREEN, BackgroundConsoleColor::CLEAR);
    }

    template<class... Args>
    static void warn(std::format_string<Args...> fmt, Args&&... args) {
        const std::string msg = std::format(fmt, std::forward<Args>(args)...);
        const std::string full = "[WARN]: " + msg + "\n";
        platform::log_output(full.c_str(), false, ForegroundConsoleColor::YELLOW, BackgroundConsoleColor::CLEAR);
    }

    template<class... Args>
    static void error(std::format_string<Args...> fmt, Args&&... args) {
        const std::string msg = std::format(fmt, std::forward<Args>(args)...);
        const std::string full = "[ERROR]: " + msg + "\n";
        platform::log_output(full.c_str(), true, ForegroundConsoleColor::RED, BackgroundConsoleColor::CLEAR);
    }

    template<class... Args>
    static void fatal(std::format_string<Args...> fmt, Args&&... args) {
        const std::string msg = std::format(fmt, std::forward<Args>(args)...);
        const std::string full = "[FATAL]: " + msg + "\n";
        platform::log_output(full.c_str(), true, ForegroundConsoleColor::WHITE, BackgroundConsoleColor::RED);
    }

private:
    static inline std::unique_ptr<log> _instance;
};

#define FATAL_ASSERT(cond, msg, ...) \
    if(!(cond)) { \
        log::fatal(msg, ##__VA_ARGS__); \
        exit(1); \
    }

#endif //BSRENDERER_LOG_H
