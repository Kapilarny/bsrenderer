//
// Created by dawid on 10/25/26.
//

#ifndef BSRENDERER_SOFTWARE_RENDERER_H
#define BSRENDERER_SOFTWARE_RENDERER_H

#include <vector>

#include "renderer/renderer.h"
#include "math/bmath.h"
#include "renderer/camera.h"

// TODO: Integrate this with framebuffer
class depth_buffer {
public:
    depth_buffer(i32 w, i32 h);
    void clear();

    double& at(i32 x, i32 y);
private:
    std::vector<double> _buffer;
    i32 w, h;
};

class software_renderer : public renderer {
public:
    explicit software_renderer(window *window);
    ~software_renderer() override;

    void begin_frame() override;
    void end_frame() override;
    void clear_screen(color clr) override;

    void draw_model(model &model) override;
    void attach_camera(camera *cam) override;

    void draw_primitive(vec4<double> clip[], color clr);
    void rasterize_triangle(vec4<double> clip[], color clr);
private:
    void _draw_dot(u32 x, u32 y, u32 color) const;
    void _draw_line(vec2<i32> p1, vec2<i32> p2, u32 color) const;

    camera* _cam{};
    framebuffer* _fb{};
    depth_buffer _depth_buffer;
};


#endif //BSRENDERER_SOFTWARE_RENDERER_H