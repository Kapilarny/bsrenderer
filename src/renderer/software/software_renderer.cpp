//
// Created by dawid on 10/25/26.
//

#include "software_renderer.h"

#include <cstring>
#include <math.h>

#include "platform/platform.h"
#include "util/log.h"
#include "util/model.h"

#include "math/bmath.h"

software_renderer::software_renderer(window *window)
    : renderer(window), _depth_buffer(depth_buffer(window->get_width(), window->get_height())) {}

software_renderer::~software_renderer() {
    // TODO: destroy shi
}

void software_renderer::begin_frame() {
    _fb = _window->begin_frame();
    _depth_buffer.clear();
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
    auto ortho = [width, height](const vec3<double> &vertex) -> vec3<i32> {
        i32 x = (vertex.x+1.0)/2.0 * (width);
        i32 y = (vertex.y+1.0)/2.0 * (height);
        i32 z = (vertex.z+1.0)/2.0 * 255;

        return {x,y,z};
    };

    auto persp = [ortho](const vec3<double>& v) -> vec3<int> {
        double c = 3.0;
        return ortho(v / (1-v.z/c));
    };

    auto rotate = [model](const vec3<double>& v) {
        // constexpr double a = M_PI/6;
        double a = model.rotation;
        const mat<3,3> R = {{std::cos(a), 0, std::sin(a), 0, 1, 0, -std::sin(a), 0, std::cos(a)}};
        return R*v;
    };

    // indices!
    srand(0);
    for (const auto& indices : model._indices) {
        const auto p1 = persp(rotate(model._vertices[indices.x]));
        const auto p2 = persp(rotate(model._vertices[indices.y]));
        const auto p3 = persp(rotate(model._vertices[indices.z]));

        auto clr = color(rand() % 0xFFFFFF);

        draw_triangle({p1, p2, p3}, clr);
    }
}

void software_renderer::draw_triangle(vec3<vec3<i32>> points, color clr) {
    auto [p1, p2, p3] = points.data;
    i32 b_xstart = std::min(p1.x, std::min(p2.x, p3.x));
    i32 b_xend = std::max(p1.x, std::max(p2.x, p3.x));
    i32 b_ystart = std::min(p1.y, std::min(p2.y, p3.y));
    i32 b_yend = std::max(p1.y, std::max(p2.y, p3.y));

    auto triangle_area = [](const vec2<i32>& a, const vec2<i32>& b, const vec2<i32>& c) -> double {
        return 0.5 * ((b.y-a.y)*(b.x+a.x)+(c.y-b.y)*(c.x+b.x)+(a.y-c.y)*(a.x+c.x));
    };
    const auto area = triangle_area(p1, p2, p3);
    if (area < 1) return; // back

    const auto w = _fb->get_width();
    const auto h = _fb->get_height();

    b_xstart = std::max(0, b_xstart);
    b_xend = std::min(w-1, b_xend);
    b_ystart = std::max(0, b_ystart);
    b_yend = std::min(h-1, b_yend);

#pragma omp parallel for
    for (int x = b_xstart; x <= b_xend; x++) {
        for (int y = b_ystart; y <= b_yend; y++) {
            const auto alpha = triangle_area({x, y}, p2, p3) / area;
            const auto beta = triangle_area({x, y}, p3, p1) / area;
            const auto gamma = triangle_area({x, y}, p1, p2) / area;

            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            float z = p1.z * alpha + p2.z * beta + p3.z * gamma;
            if (_depth_buffer.at(x, y) >= z) continue;
            _depth_buffer.at(x, y) = z;

            auto interpolated = color(z, z, z);
            _draw_dot(x, y, clr.to_argb());
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

depth_buffer::depth_buffer(i32 w, i32 h) : w(w), h(h), _buffer(w*h) {
}

void depth_buffer::clear() {
    memset(_buffer.data(), 0, w*h*sizeof(_buffer[0]));
}

float& depth_buffer::at(i32 x, i32 y) {
    return _buffer[y*w + x];
}
