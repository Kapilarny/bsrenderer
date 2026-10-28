//
// Created by dawid on 10/25/26.
//

#include "software_renderer.h"

#include <cstring>
#include <cmath>

#include "triangle_clip.h"
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

void software_renderer::attach_camera(camera *cam) {
    _cam = cam;
}

void software_renderer::clear_screen(const color clr) {
    DEBUG_ASSERT(_fb, "software_renderer::clear_screen() -> Attempt to draw without framebuffer!");
    _fb->clear_buffer(clr.to_argb());
}

void software_renderer::draw_model(model &model) {
    DEBUG_ASSERT(_fb, "software_renderer::draw_model() -> Attempt to draw without framebuffer!");
    DEBUG_ASSERT(_cam, "sofware_renderer::draw_model() -> Attempt to draw without an attached camera!");

    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    // indices!
    srand(0);
    for (const auto& indices : model._indices) {
        vec4<double> clip[3];
        for (const i32 d : {0, 1, 2}) {
            const auto [x, y, z] = model._vertices[indices[d]];
            clip[d] = _cam->_projection * _cam->_view * vec4<double>{x, y, z, 1.};
        }

        const auto clr = color(rand() % 0xFFFFFF);
        draw_primitive(clip, clr);
    }
}

void software_renderer::draw_primitive(vec4<double> clip[], color clr) {
    const auto polygon = triangle_clip::clip_triangle(clip);

    for (i32 i = 1; i + 1 < polygon.count; i++) {
        vec4<double> tri[3] = {
            polygon.vertices[0],
            polygon.vertices[i],
            polygon.vertices[i+1]
        };

        rasterize_triangle(tri, clr);
    }
}

void software_renderer::rasterize_triangle(vec4<double> clip[3], const color clr) {
    const vec4<double> ndc[3] = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };

    const auto& viewport = _cam->_viewport;

    vec2<double> screen[3] = {
        (viewport*ndc[0]).xy(), (viewport*ndc[1]).xy(), (viewport*ndc[2]).xy()
    };

    const mat<3, 3> ABC = {
        {
            screen[0].x, screen[0].y, 1.0,
            screen[1].x, screen[1].y, 1.0,
            screen[2].x, screen[2].y, 1.0,
        }
    };

    if (ABC.determinant() < 1) return; // cull + triangles less than a px

    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    const auto [b_xstart, b_xend] = std::minmax({screen[0].x, screen[1].x, screen[2].x});
    const auto [b_ystart, b_yend] = std::minmax({screen[0].y, screen[1].y, screen[2].y});

    const i32 xstart = std::max<i32>(0, b_xstart);
    const i32 xend = std::min<i32>(b_xend, width-1); // prevent overdraw

    const i32 ystart = std::max<i32>(0, b_ystart);
    const i32 yend = std::min<i32>(b_yend, height-1);

    const auto ABC_inv_transp = ABC.invert_transpose();
#pragma omp parallel for
    for (i32 x = xstart; x <= xend; x++) {
        for (i32 y = ystart; y <= yend; y++) {
            vec3 bc = ABC_inv_transp * vec3<double>{static_cast<double>(x), static_cast<double>(y), 1.0};

            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            const double z = bc * vec3<double>{ndc[0].z, ndc[1].z, ndc[2].z};

            if (z >= _depth_buffer.at(x, y)) continue;
            _depth_buffer.at(x, y) = z;

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

    // if (x >= width || y >= height) return;
    y = height-y-1;
    _fb->get_pixels()[y*width + x] = color;
}

depth_buffer::depth_buffer(i32 w, i32 h) : _buffer(w*h, std::numeric_limits<double>::max()), w(w), h(h) {
}

void depth_buffer::clear() {
    for (auto& el : _buffer) {
        el = std::numeric_limits<double>::max();
    }
}

double& depth_buffer::at(i32 x, i32 y) {
    return _buffer[y*w + x];
}
