//
// Created by dawid on 10/26/26.
//

#ifndef BSRENDERER_FRAMEBUFFER_H
#define BSRENDERER_FRAMEBUFFER_H

#include "defines.h"

class inner_framebuffer;

// TODO: make framebuffer take pixel format specification
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

#endif //BSRENDERER_FRAMEBUFFER_H