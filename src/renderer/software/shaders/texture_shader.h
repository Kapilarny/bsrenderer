//
// Created by dawid on 10/30/26.
//

#ifndef BSRENDERER_TEXTURE_SHADER_H
#define BSRENDERER_TEXTURE_SHADER_H
#include "ishader.h"
#include "renderer/software/texture.h"
#include "util/model.h"

struct texture_shader : ishader {
    texture* tex;

    struct varying {
        vec2<double> uv;
        double inv_w{};
    };

    varying varyings[3];

    vertex vertex_shader(const i32 face, const i32 vert) override {
        const auto idx = mdl->indices[face][vert];
        const auto [vx, vy, vz] = mdl->vertices[idx.vertex];
        const auto uv = mdl->uvs[idx.uv];
        const vec4<double> pos = *ctx.model_view * vec4<double>(vx, vy, vz, 1.0);

        const vec4<double> clip = *ctx.projection * pos;
        return {clip, uv };
    }

    std::pair<bool, color> fragment_shader(vec3<double> bar) const override {
        const double inv_w =
            bar.x * varyings[0].inv_w +
            bar.y * varyings[1].inv_w +
            bar.z * varyings[2].inv_w;

        const vec2<double> uv =
            (varyings[0].uv * bar.x * varyings[0].inv_w +
            varyings[1].uv * bar.y * varyings[1].inv_w +
            varyings[2].uv * bar.z * varyings[2].inv_w) / inv_w;

        return {
            false,
            tex->sample(uv)
        };
    }

    void setup_triangle(const vertex tri[3]) override {
        for (i32 i = 0; i < 3; i++) {
            varyings[i].uv = tri[i].uv;
            varyings[i].inv_w = 1.0 / tri[i].pos.w;
        }
    }
};

#endif //BSRENDERER_TEXTURE_SHADER_H