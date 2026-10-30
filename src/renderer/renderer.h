//
// Created by dawid on 10/25/26.
//

#ifndef BSRENDERER_RENDERER_H
#define BSRENDERER_RENDERER_H
#include "camera.h"
#include "defines.h"
#include "math/vec.h"

class imaterial;
class window;
class model;
class framebuffer;
struct ishader;

enum class renderer_type {
    SOFTWARE,
    OPENGL
};

struct color {
    constexpr color() = default;

    explicit constexpr color(const u32 rgb_hex) {
        a = 255;
        b = rgb_hex;
        g = rgb_hex >> 8;
        r = rgb_hex >> 16;
    }

    explicit constexpr color(const u32 r, const u32 g, const u32 b) : r(r), g(g), b(b), a(255) {}
    explicit constexpr color(const u32 r, const u32 g, const u32 b, const u32 a) : r(r), g(g), b(b), a(a) {}

    [[nodiscard]] u32 to_argb() const {
        return a << 24 | r << 16 | g << 8 | b;
    }

    constexpr u8& operator[](const u32 index) {
        switch (index) {
            case 0: return r;
            case 1: return g;
            case 2: return b;
            default: break;
        }

        return a;
    }

    u8 a{}, r{}, g{}, b{};
};

class renderer {
public:
    virtual ~renderer() = default;
    explicit renderer(window* window);

    static std::unique_ptr<renderer> create_renderer(window* window, renderer_type type);

    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void clear_screen(color clr) = 0;

    virtual void attach_camera(camera* cam) = 0;

    virtual void draw_model(model& model, imaterial& material) = 0;
    virtual void blit_text(const std::string& str, u32 x, u32 y, color clr) = 0;
protected:
    window* _window;
};

#endif //BSRENDERER_RENDERER_H
