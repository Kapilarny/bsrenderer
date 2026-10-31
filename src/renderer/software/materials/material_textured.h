//
// Created by dawid on 10/30/26.
//

#ifndef BSRENDERER_MATERIAL_TEXTURED_H
#define BSRENDERER_MATERIAL_TEXTURED_H
#include <utility>

#include "imaterial.h"
#include "renderer/software/texture.h"
#include "renderer/software/shaders/texture_shader.h"

struct material_textured : imaterial {
    texture tex;
    texture_shader shader;
    explicit material_textured(texture texture) : tex(std::move(texture)), shader(&tex) {
    }

    ishader *get_shader() override {
        return &shader;
    }
};

#endif //BSRENDERER_MATERIAL_TEXTURED_H