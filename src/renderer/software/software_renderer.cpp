//
// Created by dawid on 10/25/26.
//

#include "software_renderer.h"

#include <cstring>
#include <cmath>

#include "triangle_clip.h"
#include "font/font.h"
#include "materials/material_textured.h"
#include "platform/platform.h"
#include "util/log.h"
#include "util/model.h"

#include "math/bmath.h"
#include "shaders/flat_shader.h"
#include "shaders/texture_shader.h"

#include "vertex.h"
#include "tracy/Tracy.hpp"

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

void software_renderer::draw_model(model &model, imaterial& material) {
    ZoneScoped;

    DEBUG_ASSERT(_fb, "software_renderer::draw_model() -> Attempt to draw without framebuffer!");
    DEBUG_ASSERT(_cam, "sofware_renderer::draw_model() -> Attempt to draw without an attached camera!");

    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    auto shader = material.get_shader();
    shader->setup(model, {&_cam->_view, &_cam->_projection});

    // indices!
    for (i32 face = 0; face < model.indices.size(); face++) {
        triangle tri = {
            shader->vertex_shader(face, 0),
            shader->vertex_shader(face, 1),
            shader->vertex_shader(face, 2)
        };

        draw_primitive(tri, shader);
    }
}

void software_renderer::blit_text(const std::string &str, u32 x, u32 y, color clr) {
    auto font = font_font;

    const auto find_glyph = [font](char c) -> const glyph_t* {
        for (i32 i = 0; i < font.glyph_count; i++) {
            if (font.glyph[i].cp == c) return &font.glyph[i];
        }

        return nullptr;
    };

    for (const char c : str) {
        const auto glyph = find_glyph(c);

        if (!glyph) {
            log::error("Couldn't find glyph for: {}", c);
            continue;
        }

        for (auto gx = 0; gx < glyph->w; gx++) {
            for (auto gy = 0; gy < glyph->h; gy++) {
                const u32 dot = reinterpret_cast<const u32*>(glyph->data)[(glyph->h - gy - 1) * glyph->w + gx];
                const color dot_clr = color(dot);
                if (dot_clr.b == 255) {
                    _draw_dot(gx + x, gy + y, clr.to_argb());
                }
            }
        }

        x += glyph->advance;
    }
}

void software_renderer::draw_primitive(triangle clip, ishader* shader) {
    ZoneScoped;
    const auto polygon = triangle_clip::clip_triangle(clip);

    for (i32 i = 1; i + 1 < polygon.count; i++) {
        vertex tri[3] = {
            polygon.vertices[0],
            polygon.vertices[i],
            polygon.vertices[i+1],
        };

        shader->setup_triangle(tri);

        vec4<double> clip[3] = {
            polygon.vertices[0].pos,
            polygon.vertices[i].pos,
            polygon.vertices[i+1].pos
        };

        rasterize_triangle(clip, shader);
    }
}

void software_renderer::rasterize_triangle(const vec4<double> clip[4], const ishader* shader) {
    ZoneScoped;

    const vec4<double> ndc[3] = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };

    const auto& viewport = _cam->_viewport;
    vec2<double> screen[3] = {
        (viewport*ndc[0]).xy(), (viewport*ndc[1]).xy(), (viewport*ndc[2]).xy()
    };

    const auto edge_function = [](const vec2<double>& a, const vec2<double>& b, const vec2<double>& p) {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    const double area = -edge_function(screen[0], screen[1], screen[2]);
    if (area < 1.0) return;

    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    const auto [b_xstart, b_xend] = std::minmax({screen[0].x, screen[1].x, screen[2].x});
    const auto [b_ystart, b_yend] = std::minmax({screen[0].y, screen[1].y, screen[2].y});

    const i32 xstart = std::max<i32>(0, b_xstart);
    const i32 xend = std::min<i32>(b_xend, width-1); // prevent overdraw

    const i32 ystart = std::max<i32>(0, b_ystart);
    const i32 yend = std::min<i32>(b_yend, height-1);

    const double inv_area = 1.0 / area;
    const vec3<double> dx = {
        screen[1].y - screen[2].y,
        screen[2].y - screen[0].y,
        screen[0].y - screen[1].y
    };

    const vec3<double> ndc_z = {
        ndc[0].z, ndc[1].z, ndc[2].z
    };

    const auto pixels = _fb->get_pixels();

    // loop over y for less cache misses
#pragma omp parallel for
    for (i32 y = ystart; y <= yend; y++) {
        const vec2<double> start = {
            static_cast<double>(xstart),
            static_cast<double>(y)
        };

        vec3<double> edge = {
            -edge_function(screen[1], screen[2], start),
            -edge_function(screen[2], screen[0], start),
            -edge_function(screen[0], screen[1], start)
        };

        for (i32 x = xstart; x <= xend; x++) {
            if (edge.x >= 0 && edge.y >= 0 && edge.z >= 0) {
                const auto bc = edge * inv_area;
                const double z = bc * ndc_z;

                if (z < _depth_buffer.at(x, y)) {
                    auto [discard, clr] = shader->fragment_shader(bc);

                    if (!discard) {
                        _depth_buffer.at(x, y) = z;

                        // _draw_dot(x, y, clr.to_argb());
                        pixels[(height-y-1)*width+x] = clr.to_argb(); // manual inline of this seems to give a lil boost
                    }
                }
            }

            edge += dx;
        }
    }
}

void software_renderer::_draw_dot(const u32 x, u32 y, const u32 color) const {
    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    y = height-y-1;
    _fb->get_pixels()[y*width + x] = color;
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
