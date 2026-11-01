//
// Created by dawid on 10/30/26.
//

#ifndef BSRENDERER_FLAT_SHADER_H
#define BSRENDERER_FLAT_SHADER_H
#include "ishader.h"
#include "util/model.h"

struct flat_shader : ishader {
    color clr{};

    vertex vertex_shader(const i32 face, const i32 vert) override {
        const auto [vx, vy, vz] = mdl->get_vertex(face, vert);
        vec4<float> pos = *ctx.model_view * vec4<float>(vx, vy, vz, 1.0);

        return {*ctx.projection * pos};
    }

    std::pair<bool, color> fragment_shader(const vec3<float>& bar) const override {
        return {false, clr};
    }
};

#endif //BSRENDERER_FLAT_SHADER_H