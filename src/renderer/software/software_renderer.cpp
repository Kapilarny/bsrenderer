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
        i32 x = (vertex.x+1.0)/2.0 * (width);
        i32 y = (vertex.y+1.0)/2.0 * (height);

        return vec2<i32>(x,y);
    };

    // indices!
    srand(0);
    for (const auto indices : model._indices) {
        const auto p1 = vertex_to_screen(model._vertices[indices.x]);
        const auto p2 = vertex_to_screen(model._vertices[indices.y]);
        const auto p3 = vertex_to_screen(model._vertices[indices.z]);

        // _draw_line(p1, p2, clr_red.to_argb());
        // _draw_line(p2, p3, clr_red.to_argb());
        // _draw_line(p3, p1, clr_red.to_argb());

        auto clr = color(rand() % 0xFFFFFF);

        draw_triangle({p1, p2, p3}, clr);
    }
}

void software_renderer::draw_triangle(vec3<vec2<i32>> points, color clr) {
    auto [p1, p2, p3] = points;
    const i32 b_xstart = std::min(p1.x, std::min(p2.x, p3.x));
    const i32 b_xend = std::max(p1.x, std::max(p2.x, p3.x));
    const i32 b_ystart = std::min(p1.y, std::min(p2.y, p3.y));
    const i32 b_yend = std::max(p1.y, std::max(p2.y, p3.y));

    const vec3<i32> d = {25, 100, 255};

    auto triangle_area = [](const vec2<i32> a, const vec2<i32> b, const vec2<i32> c) -> double {
        return 0.5 * ((b.y-a.y)*(b.x+a.x)+(c.y-b.y)*(c.x+b.x)+(a.y-c.y)*(a.x+c.x));
    };
    const auto area = triangle_area(p1, p2, p3);
    if (area < 1) return; // back

#pragma omp parallel for
    for (int x = b_xstart; x <= b_xend; x++) {
        for (int y = b_ystart; y <= b_yend; y++) {
            const auto alpha = triangle_area({x, y}, p2, p3) / area;
            const auto beta = triangle_area({x, y}, p3, p1) / area;
            const auto gamma = triangle_area({x, y}, p1, p2) / area;

            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            if (alpha >= 0.1 && beta >= 0.1 && gamma >= 0.1) continue;

            auto interpolated = color(clr.r * alpha, clr.g * beta, clr.g * gamma);
            _draw_dot(x, y, interpolated.to_argb());
        }
    }
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


void software_renderer::_draw_dot(const u32 x, u32 y, const u32 color) const {
    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    if (x >= width || y >= height) return;
    y = height-y-1;
    _fb->get_pixels()[y*_fb->get_width() + x] = color;
}