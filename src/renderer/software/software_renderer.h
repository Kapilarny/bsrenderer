//
// Created by dawid on 10/25/26.
//

#ifndef BSRENDERER_SOFTWARE_RENDERER_H
#define BSRENDERER_SOFTWARE_RENDERER_H

#include <vector>

#include "vertex.h"
#include "renderer/renderer.h"
#include "renderer/camera.h"
#include "util/thread_pool.h"


// TODO: Integrate this with framebuffer
class depth_buffer {
public:
    depth_buffer(i32 w, i32 h);
    void clear();

    float& at(i32 x, i32 y);
private:
    std::vector<float> _buffer;
    i32 w, h;
};

struct raster_triangle {
    u32 min_x{}, min_y{}, max_x{}, max_y{}; // bounding box
    float inv_area{};
    vec2<float> screen[3];
    vec3<float> ndc_z;
    vec3<float> edge_dx;
    vertex tri[3];
};

class software_renderer : public renderer {
public:
    explicit software_renderer(window *window);
    ~software_renderer() override;

    void begin_frame() override;
    void end_frame() override;
    void clear_screen(color clr) override;

    void set_projection_mode(bool orthogonal) override;

    void draw_model(const model* mdl, imaterial* material, const mat<4,4>& mat) override;

    void attach_camera(camera *cam) override;

    void draw_primitive(triangle clip, ishader *shader);
    void rasterize_triangle(const vertex tri[3], ishader *shader);

    std::vector<raster_triangle> create_raster_triangles(const model *mdl, imaterial *material, const mat<4, 4> &mat);
    void clip_triangle_and_prepare_raster(const triangle tri, std::vector<raster_triangle>& tris);
    void prepare_triangle_for_raster(const vertex clipped[3], std::vector<raster_triangle>& tris) const;
    void rasterize_tiled_triangle(const raster_triangle &tri, u32 tile_x, u32 tile_y, ishader *shader);
private:
    constexpr static u32 CELL_SIZE = 32;

    void _draw_dot(u32 x, u32 y, u32 color) const;
    void _draw_line(vec2<i32> p1, vec2<i32> p2, u32 color) const;

    mat<4, 4> _projection;
    bool _orthogonal = false;

    camera* _cam{};
    framebuffer* _fb{};
    depth_buffer _depth_buffer;
    thread_pool _thread_pool;
};


#endif //BSRENDERER_SOFTWARE_RENDERER_H