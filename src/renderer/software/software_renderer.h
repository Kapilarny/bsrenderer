//
// Created by dawid on 10/25/26.
//

#ifndef BSRENDERER_SOFTWARE_RENDERER_H
#define BSRENDERER_SOFTWARE_RENDERER_H
#include "renderer/renderer.h"

class software_renderer : public renderer {
public:
    explicit software_renderer(window *window);
    virtual ~software_renderer();

    void begin_frame() override;
    void end_frame() override;
    void clear_screen(color clr) override;

    void draw_model(model &model) override;
private:
    void _draw_dot(u32 x, u32 y, u32 color) const;
    void _draw_line(vec2<i32> p1, vec2<i32> p2, u32 color) const;

    framebuffer* _fb{};
};


#endif //BSRENDERER_SOFTWARE_RENDERER_H