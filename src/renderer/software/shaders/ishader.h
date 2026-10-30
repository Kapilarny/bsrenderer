//
// Created by dawid on 10/29/26.
//

#ifndef BSRENDERER_ISHADER_H
#define BSRENDERER_ISHADER_H
#include <utility>

#include "defines.h"
#include "math/vec.h"
#include "renderer/renderer.h"
#include "renderer/software/software_renderer.h"

#include "renderer/software/vertex.h"

class model;
struct color;

struct shader_ctx {
    const mat<4, 4> *model_view, *projection;
};

struct ishader {
    virtual ~ishader() = default;

    void setup(const model& m, const shader_ctx& c) {
        mdl = &m;
        ctx = c;
    }

    virtual void setup_triangle(const vertex tri[3]) {}

    virtual vertex vertex_shader(i32 face, i32 vert) = 0;
    virtual std::pair<bool, color> fragment_shader(vec3<double> bar) const = 0;

    const model* mdl{};
    shader_ctx ctx{};
};

#endif //BSRENDERER_ISHADER_H