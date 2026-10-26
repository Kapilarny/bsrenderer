//
// Created by dawid on 10/23/26.
//

#include "defines.h"
#ifdef BSRENDERER_LINUX

#include <iostream>

#include "platform/platform.h"

void platform::log_output(const char *str, bool is_error, foreground_console_color foreground, background_console_color background) {
    constexpr const char* foreground_codes[] = {
        "\033[39m",
        "\033[97m",
        "\033[32m",
        "\033[31m",
        "\033[33m"
    };

    constexpr const char* background_codes[] = {
        "\033[49m",
        "\033[41m",
        "\033[43m",
    };

    auto& stream = is_error ? std::cerr : std::cout;
    stream << foreground_codes[static_cast<int>(foreground)];
    stream << background_codes[static_cast<int>(background)];
    stream << str;
}

#endif
