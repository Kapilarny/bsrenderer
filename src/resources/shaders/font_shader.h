//
// Created by dawid on 11/4/26.
//

#ifndef BSRENDERER_FONT_SHADER_H
#define BSRENDERER_FONT_SHADER_H

#include "ishader.h"
#include "resources/texture.h"
#include "util/model.h"

struct font_shader : ishader {
    texture* tex;
    color text_color;

    explicit font_shader(texture* tex, const color text_color) : tex(tex), text_color(text_color) {}

    struct varying {
        vec2<float> uv;
        float inv_w{};
    };

    varying varyings[3];

    std::unique_ptr<ishader> clone() const override {
        return std::make_unique<font_shader>(tex, text_color);
    }

    vertex vertex_shader(const i32 face, const i32 vert) override {
        const auto [vx, vy, vz] = mdl->get_vertex(face, vert);
        const auto uv = mdl->get_uv(face, vert);
        const vec4<float> pos = *ctx.model_view * vec4<float>(vx, vy, vz, 1.0);

        const vec4<float> clip = *ctx.projection * pos;
        return {clip, uv };
    }

    std::pair<bool, color> fragment_shader(const vec3<float>& bar) const override {
        const float inv_w =
            bar.x * varyings[0].inv_w +
            bar.y * varyings[1].inv_w +
            bar.z * varyings[2].inv_w;

        const vec2<float> uv =
            (varyings[0].uv * bar.x * varyings[0].inv_w +
            varyings[1].uv * bar.y * varyings[1].inv_w +
            varyings[2].uv * bar.z * varyings[2].inv_w) / inv_w;

        const auto mask = tex->sample(uv);

        return {
            false,
            color(text_color.r, text_color.g, text_color.b, text_color.a * (mask.a / 255.f)),
        };
    }

    bool write_depth() const override {
        return false;
    }

    void setup_triangle(const vertex tri[3]) override {
        for (i32 i = 0; i < 3; i++) {
            varyings[i].uv = tri[i].uv;
            varyings[i].inv_w = 1.0f / tri[i].pos.w;
        }
    }
};

#endif //BSRENDERER_FONT_SHADER_H