//
// Created by dawid on 10/23/26.
//

#ifndef BSRENDERER_PLATFORM_H
#define BSRENDERER_PLATFORM_H
#include <memory>

#include "defines.h"

#include "window.h"
#include "framebuffer.h"

enum class foreground_console_color {
    CLEAR = 0,
    WHITE,
    GREEN,
    RED,
    YELLOW
};

enum class background_console_color {
    CLEAR = 0,
    RED,
    YELLOW
};

class platform {
public:
    static void init();

    static void log_output(const char* str, bool is_error,
        foreground_console_color foreground,
        background_console_color background);
};

#endif //BSRENDERER_PLATFORM_H