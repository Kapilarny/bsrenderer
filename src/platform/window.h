//
// Created by dawid on 10/26/26.
//

#ifndef BSRENDERER_WINDOW_H
#define BSRENDERER_WINDOW_H

#include <string>
#include "defines.h"
#include "input.h"

class inner_window;
class framebuffer;

struct window_options {
    i16 width, height;
    bool resizable;
};

class window {
public:
    window(const std::string& name, window_options opts);
    ~window();

    [[nodiscard]] i32 get_width() const;
    [[nodiscard]] i32 get_height() const;

    framebuffer *begin_frame();
    void end_frame();

    void capture_mouse();
    void release_mouse();

    [[nodiscard]] bool should_close() const;
    [[nodiscard]] input& get_input();
private:
    void _run_preframe_events();
    void _run_postframe_events();

    framebuffer* _framebuffers[2]{};
    u64 _frame_counter = 0;
    bool _drawing_frame = false;
    input _input;

    inner_window* _inner;
};

#endif //BSRENDERER_WINDOW_H