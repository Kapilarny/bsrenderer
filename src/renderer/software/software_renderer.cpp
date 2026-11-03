//
// Created by dawid on 10/25/26.
//

#include "software_renderer.h"

#include <cstring>
#include <cmath>

#include "triangle_clip.h"
#include "resources/materials/material_textured.h"
#include "platform/platform.h"
#include "util/log.h"
#include "util/model.h"

#include "math/bmath.h"
#include "resources/shaders/flat_shader.h"
#include "resources/shaders/texture_shader.h"

#include "vertex.h"
#include "tracy/Tracy.hpp"
#include "util/thread_pool.h"

software_renderer::software_renderer(window *window)
    : renderer(window),
    _depth_buffer(depth_buffer(window->get_width(), window->get_height())),
    _thread_pool(std::thread::hardware_concurrency()) {}

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
    ZoneScoped;

    DEBUG_ASSERT(_fb, "software_renderer::clear_screen() -> Attempt to draw without framebuffer!");
    _fb->clear_buffer(clr.to_argb());
}

void software_renderer::draw_model(const model* mdl, imaterial* material, const mat<4,4>& mat) {
    ZoneScoped;

    DEBUG_ASSERT(_fb, "software_renderer::draw_model() -> Attempt to draw without framebuffer!");
    DEBUG_ASSERT(_cam, "sofware_renderer::draw_model() -> Attempt to draw without an attached camera!");

    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    auto allocated_workers = _thread_pool.get_allocated_thread_count();

    auto tris = create_raster_triangles(mdl, material, mat);

    // Bin
    const u32 cells_x = (width + CELL_SIZE - 1) / CELL_SIZE;
    const u32 cells_y = (height + CELL_SIZE - 1) / CELL_SIZE;

    std::vector<std::vector<u32>> bins(cells_x * cells_y);
    for (u32 i = 0; i < tris.size(); i++) {
        const auto& tri = tris[i];
        const u32 cell_min_x = tri.min_x / CELL_SIZE;
        const u32 cell_max_x = tri.max_x / CELL_SIZE;
        const u32 cell_min_y = tri.min_y / CELL_SIZE;
        const u32 cell_max_y = tri.max_y / CELL_SIZE;

        for (u32 y = cell_min_y; y <= cell_max_y; y++) {
            for (u32 x = cell_min_x; x <= cell_max_x; x++) {
                bins[y*cells_x+x].push_back(i);
            }
        }
    }

    // Batch draw triangles
    {
        const u32 worker_count = std::min<u32>(allocated_workers, bins.size()); // this is pretty much always true
        for (u32 worker = 0; worker < worker_count; worker++) {
            _thread_pool.enqueue([this, &material, worker_count, &bins, worker, cells_x, &tris]() {
                const auto shader_inst = material->get_shader()->clone();

                const u32 base = bins.size() / worker_count;
                const u32 remainder = bins.size() % worker_count;

                const u32 beg = worker * base + std::min(worker, remainder);
                const u32 end = beg + base + (worker < remainder ? 1 : 0);

                for (u32 bin = beg; bin < end; bin++) {
                    const u32 y = bin / cells_x;
                    const u32 x = bin % cells_x;

                    const auto& tris_idx = bins[bin];

                    for (const auto& idx : tris_idx) {
                        raster_triangle& tri = tris[idx];
                        rasterize_tiled_triangle(tri, x, y, shader_inst.get());
                    }
                }
            });
        }

        _thread_pool.wait_blocking();
    }
}

void software_renderer::prepare_triangle_for_raster(const vertex clipped[3], std::vector<raster_triangle>& tris) const {
    const auto width = _fb->get_width();
    const auto height = _fb->get_height();
    const auto viewport = _cam->_viewport;

    vec4<float> ndc[3] =
            { clipped[0].pos/clipped[0].pos.w, clipped[1].pos/clipped[1].pos.w, clipped[2].pos/clipped[2].pos.w };

    vec2<float> screen[3] = {
        (viewport*ndc[0]).xy(), (viewport*ndc[1]).xy(), (viewport*ndc[2]).xy()
    };

    const auto edge_function = [](const vec2<float>& a, const vec2<float>& b, const vec2<float>& p) {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    const float area = -edge_function(screen[0], screen[1], screen[2]);
    if (area < 1.0) return;

    const auto [b_xstart, b_xend] = std::minmax({screen[0].x, screen[1].x, screen[2].x});
    const auto [b_ystart, b_yend] = std::minmax({screen[0].y, screen[1].y, screen[2].y});
    const i32 xstart = std::max<i32>(0, b_xstart);
    const i32 xend = std::min<i32>(b_xend, width-1); // prevent overdraw

    const i32 ystart = std::max<i32>(0, b_ystart);
    const i32 yend = std::min<i32>(b_yend, height-1);

    raster_triangle r_tri;

    r_tri.ndc_z = {ndc[0].z, ndc[1].z, ndc[2].z};
    r_tri.screen[0] = screen[0];
    r_tri.screen[1] = screen[1];
    r_tri.screen[2] = screen[2];
    r_tri.inv_area = 1.0f/area;
    r_tri.min_x = xstart;
    r_tri.max_x = xend;
    r_tri.min_y = ystart;
    r_tri.max_y = yend;
    r_tri.tri[0] = clipped[0];
    r_tri.tri[1] = clipped[1];
    r_tri.tri[2] = clipped[2];

    r_tri.edge_dx = {
        screen[1].y - screen[2].y,
        screen[2].y - screen[0].y,
        screen[0].y - screen[1].y
    };

    tris.emplace_back(r_tri);
}

void software_renderer::clip_triangle_and_prepare_raster(const triangle tri, std::vector<raster_triangle>& tris) {
    // ZoneScoped;
    const auto polygon = triangle_clip::clip_triangle(tri);

    for (i32 i = 1; i + 1 < polygon.count; i++) {
        const vertex clipped[3] = {
            polygon.vertices[0],
            polygon.vertices[i],
            polygon.vertices[i+1],
        };

        prepare_triangle_for_raster(clipped, tris);
    }
}

std::vector<raster_triangle> software_renderer::create_raster_triangles
    (const model *mdl, imaterial* material, const mat<4, 4> &mat) {
    // ZoneScoped;

    auto shader = material->get_shader();
    auto model_view = _cam->_view * mat;

    shader->setup(mdl, {&model_view, &_cam->_projection});

    const u32 face_count = mdl->face_count();
    const u32 worker_count = std::min<u32>(_thread_pool.get_allocated_thread_count(), face_count);
    std::vector<std::vector<raster_triangle>> local_tris(worker_count);

    const u32 base = face_count / worker_count;
    const u32 rem = face_count % worker_count;

    for (u32 worker = 0; worker < worker_count; worker++) {
        const u32 beg = worker * base + std::min<u32>(worker, rem);
        const u32 end = beg + base + (worker < rem ? 1 : 0);

        auto& out = local_tris[worker];

        _thread_pool.enqueue([this, shader, &out, beg, end] {
            for (i32 face = static_cast<i32>(beg); face < end; face++) {
               const triangle tri = {
                   shader->vertex_shader(face, 0),
                   shader->vertex_shader(face, 1),
                   shader->vertex_shader(face, 2)
               };

               clip_triangle_and_prepare_raster(tri, out);
           }
        });
    }

    _thread_pool.wait_blocking();

    std::vector<raster_triangle> tris;
    u64 total = 0;
    for (const auto& local : local_tris) {
        total += local.size();
    }

    tris.reserve(total);

    for (const auto& local : local_tris) {
        tris.insert(tris.end(), std::make_move_iterator(local.begin()), std::make_move_iterator(local.end()));
    }

    return tris;
}

void software_renderer::rasterize_tiled_triangle(const raster_triangle &tri, u32 tile_x, u32 tile_y, ishader* shader) {
    const auto pixels = _fb->get_pixels();

    const u32 tile_x_start = tile_x * CELL_SIZE;
    const u32 tile_x_end = tile_x_start + CELL_SIZE;
    const u32 tile_y_start = tile_y * CELL_SIZE;
    const u32 tile_y_end = tile_y_start + CELL_SIZE;

    // clamp
    const u32 x_start = std::max(tri.min_x, tile_x_start);
    const u32 x_end = std::min(tri.max_x, tile_x_end);
    const u32 y_start = std::max(tri.min_y, tile_y_start);
    const u32 y_end = std::min(tri.max_y, tile_y_end);

    const auto edge_function = [](const vec2<float>& a, const vec2<float>& b, const vec2<float>& p) {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    shader->setup_triangle(tri.tri);

    for (u32 y = y_start; y <= y_end; y++) {
        const vec2<float> start = {
            static_cast<float>(x_start),
            static_cast<float>(y)
        };

        vec3<float> edge = {
            -edge_function(tri.screen[1], tri.screen[2], start),
            -edge_function(tri.screen[2], tri.screen[0], start),
            -edge_function(tri.screen[0], tri.screen[1], start)
        };

        for (u32 x = x_start; x <= x_end; x++) {
            if (edge.x >= 0 && edge.y >= 0 && edge.z >= 0) {
                const auto bc = edge * tri.inv_area;
                const float z = bc * tri.ndc_z;
                const auto [discard, clr] = shader->fragment_shader(bc);
                if (discard) continue;
                if (z < _depth_buffer.at(x, y)) {
                    _depth_buffer.at(x, y) = z;

                    pixels[(height-y-1)*width+x] = clr.to_argb();
                }
            }

            edge += tri.edge_dx;
        }
    }
}

void software_renderer::draw_primitive(triangle clip, ishader* shader) {
    // ZoneScoped;
    const auto polygon = triangle_clip::clip_triangle(clip);

    for (i32 i = 1; i + 1 < polygon.count; i++) {
        const vertex tri[3] = {
            polygon.vertices[0],
            polygon.vertices[i],
            polygon.vertices[i+1],
        };

        rasterize_triangle(tri, shader);
    }
}

void software_renderer::rasterize_triangle(const vertex tri[3], ishader* shader) {
    // ZoneScoped;
    const vec4<float> ndc[3] = { tri[0].pos/tri[0].pos.w, tri[1].pos/tri[1].pos.w, tri[2].pos/tri[2].pos.w };

    const auto& viewport = _cam->_viewport;
    vec2<float> screen[3] = {
        (viewport*ndc[0]).xy(), (viewport*ndc[1]).xy(), (viewport*ndc[2]).xy()
    };

    const auto edge_function = [](const vec2<float>& a, const vec2<float>& b, const vec2<float>& p) {
        return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
    };

    const float area = -edge_function(screen[0], screen[1], screen[2]);
    if (area < 1.0) return;

    shader->setup_triangle(tri);

    const auto width = _fb->get_width();
    const auto height = _fb->get_height();

    const auto [b_xstart, b_xend] = std::minmax({screen[0].x, screen[1].x, screen[2].x});
    const auto [b_ystart, b_yend] = std::minmax({screen[0].y, screen[1].y, screen[2].y});

    const i32 xstart = std::max<i32>(0, b_xstart);
    const i32 xend = std::min<i32>(b_xend, width-1); // prevent overdraw

    const i32 ystart = std::max<i32>(0, b_ystart);
    const i32 yend = std::min<i32>(b_yend, height-1);

    const float inv_area = 1.0 / area;
    const vec3<float> dx = {
        screen[1].y - screen[2].y,
        screen[2].y - screen[0].y,
        screen[0].y - screen[1].y
    };

    const vec3<float> ndc_z = {
        ndc[0].z, ndc[1].z, ndc[2].z
    };

    const auto pixels = _fb->get_pixels();

    // loop over y for less cache misses
#pragma omp parallel for
    for (i32 y = ystart; y <= yend; y++) {
        const vec2<float> start = {
            static_cast<float>(xstart),
            static_cast<float>(y)
        };

        vec3<float> edge = {
            -edge_function(screen[1], screen[2], start),
            -edge_function(screen[2], screen[0], start),
            -edge_function(screen[0], screen[1], start)
        };

        for (i32 x = xstart; x <= xend; x++) {
            if (edge.x >= 0 && edge.y >= 0 && edge.z >= 0) {
                const auto bc = edge * inv_area;
                const float z = bc * ndc_z;

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

depth_buffer::depth_buffer(i32 w, i32 h) : _buffer(w*h, std::numeric_limits<float>::max()), w(w), h(h) {
}

void depth_buffer::clear() {
    ZoneScoped;
    std::ranges::fill(_buffer ,std::numeric_limits<float>::max());
}

float& depth_buffer::at(i32 x, i32 y) {
    return _buffer[y*w + x];
}
