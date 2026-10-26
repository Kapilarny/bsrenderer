//
// Created by dawid on 10/25/26.
//

#ifndef BSRENDERER_SOFTWARE_RENDERER_H
#define BSRENDERER_SOFTWARE_RENDERER_H

#include <vector>

#include "renderer/renderer.h"
#include "math/bmath.h"

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

class software_renderer : public renderer {
public:
    explicit software_renderer(window *window);
    virtual ~software_renderer();

    void begin_frame() override;
    void end_frame() override;
    void clear_screen(color clr) override;

    void draw_model(model &model) override;
    void draw_triangle(vec3<vec3<i32>> points, color clr) override;
private:
    void _draw_dot(u32 x, u32 y, u32 color) const;
    void _draw_line(vec2<i32> p1, vec2<i32> p2, u32 color) const;

    framebuffer* _fb{};
    depth_buffer _depth_buffer;
};


#endif //BSRENDERER_SOFTWARE_RENDERER_H