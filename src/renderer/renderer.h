//
// Created by dawid on 10/25/26.
//

#ifndef BSRENDERER_RENDERER_H
#define BSRENDERER_RENDERER_H
#include "defines.h"

class window;
class model;
class framebuffer;

enum class renderer_type {
    SOFTWARE,
    OPENGL
};

struct color {
    explicit constexpr color(const u32 rgb_hex) {
        a = 255;
        b = rgb_hex;
        g = rgb_hex >> 8;
        r = rgb_hex >> 16;
    }

    explicit constexpr color(const u32 r, const u32 g, const u32 b) : r(r), g(g), b(b), a(255) {}

    [[nodiscard]] u32 to_argb() const {
        return a << 24 | r << 16 | g << 8 | b;
    }

    u8 a{}, r{}, g{}, b{};
};

class renderer {
public:
    virtual ~renderer() = default;
    explicit renderer(window* window);

    static renderer* create_renderer(window* window, renderer_type type);

    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void clear_screen(color clr) = 0;

    virtual void draw_model(model& model) = 0;
    virtual void draw_triangle(vec3<vec2<i32>> points, color clr) = 0;
protected:
    window* _window;
};

#endif //BSRENDERER_RENDERER_H
