//
// Created by dawid on 10/23/26.
//

#ifndef BSRENDERER_PLATFORM_H
#define BSRENDERER_PLATFORM_H
#include <memory>

#include "defines.h"

class inner_framebuffer;
class inner_window;
class inner_platform;

class framebuffer {
public:
    void clear_buffer(u32 background) const;

    [[nodiscard]] u32* get_pixels() const;

    [[nodiscard]] i32 get_height() const;
    [[nodiscard]] i32 get_width() const;
private:
    framebuffer(i32 width, i32 height);

    friend class inner_window;
    friend class window;
    inner_framebuffer* _inner;
};

struct window_options {
    i16 width, height;
    bool resizable;
};

class window {
public:
    window(const std::string& name, window_options opts);
    ~window();

    framebuffer *begin_frame();
    void end_frame();

    [[nodiscard]] bool should_close() const;
private:
    void _run_preframe_events();
    void _run_postframe_events();

    framebuffer* _framebuffers[2];
    u64 _frame_counter = 0;
    bool _drawing_frame = false;

    inner_window* _inner;
};

enum class ForegroundConsoleColor {
    CLEAR = 0,
    WHITE,
    GREEN,
    RED,
    YELLOW
};

enum class BackgroundConsoleColor {
    CLEAR = 0,
    RED,
    YELLOW
};

class platform {
public:
    static void init();

    static void log_output(const char* str, bool is_error,
        ForegroundConsoleColor foreground,
        BackgroundConsoleColor background);
};

#endif //BSRENDERER_PLATFORM_H