//
// Created by dawid on 10/25/26.
//

#include "software_renderer.h"

#include "platform/platform.h"
#include "util/log.h"
#include "util/model.h"

software_renderer::software_renderer(window *window) : renderer(window) {
}

software_renderer::~software_renderer() {
    // TODO: destroy shi
}

void software_renderer::begin_frame() {
    _fb = _window->begin_frame();
    // _fb->clear_buffer(0xFF000000);
}

void software_renderer::end_frame() {
    _window->end_frame();
    _fb = nullptr; // invalidate framebuffer
}

void software_renderer::clear_screen(const color clr) {
    DEBUG_ASSERT(_fb, "software_renderer::clear_screen() -> Attempt to draw without framebuffer!");
    _fb->clear_buffer(clr.to_argb());
}

void software_renderer::draw_model(model &model) {
    DEBUG_ASSERT(_fb, "software_renderer::draw_model() -> Attempt to draw without framebuffer!");

    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    constexpr auto clr_white = color(0xFFFFFF);
    constexpr auto clr_red = color(0xFF0000);

    // Coords are between [-1, 1] so we have to normalize them
    auto vertex_to_screen = [width, height](const vec3<double> vertex) -> vec2<i32> {
        i32 x = (vertex.x+1.0)/2.0 * (width-1);
        i32 y = (vertex.y+1.0)/2.0 * (height-1);
        y = height-1-y; // flip y

        return vec2<i32>(x,y);
    };

    // indices!
    for (const auto indices : model._indices) {
        const auto p1 = vertex_to_screen(model._vertices[indices.x]);
        const auto p2 = vertex_to_screen(model._vertices[indices.y]);
        const auto p3 = vertex_to_screen(model._vertices[indices.z]);

        _draw_line(p1, p2, clr_red.to_argb());
        _draw_line(p2, p3, clr_red.to_argb());
        _draw_line(p3, p1, clr_red.to_argb());
    }

    for (const auto vertex : model._vertices) {
        const auto [x, y] = vertex_to_screen(vertex);
        _draw_dot(x, y, clr_white.to_argb());
    }
}

void software_renderer::_draw_dot(const u32 x, const u32 y, const u32 color) const {
    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    DEBUG_ASSERT(x < width && y < height, "software_renderer::_draw_dot() -> Out of bounds dot draw");
    _fb->get_pixels()[y*_fb->get_width() + x] = color;
}

void software_renderer::_draw_line(vec2<i32> p1, vec2<i32> p2, u32 color) const {
    const bool steep = std::abs(p1.x - p2.x) < std::abs(p1.y - p2.y);
    if (steep) {
        // if steep transpose
        std::swap(p1.x, p1.y);
        std::swap(p2.x, p2.y);
    }

    if (p1.x > p2.x) {
        std::swap(p1, p2);
    }

    int y = p1.y;
    int error = 0;
    for (i32 x = p1.x; x < p2.x; x++) {
        // should get unrolled into 2 loops when compiled on release
        if (steep) {
            _draw_dot(y, x, color); // de-transpose
        } else {
            _draw_dot(x, y, color);
        }

        error += 2 * std::abs(p2.y - p1.y);
        y += (p2.y > p1.y ? 1 : -1) * (error > p2.x - p1.x);
        error -= 2 * (p2.x - p1.x) * (error > p2.x - p1.x);
    }
}
