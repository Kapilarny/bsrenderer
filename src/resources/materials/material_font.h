//
// Created by dawid on 11/4/26.
//

#ifndef BSRENDERER_MATERIAL_FONT_H
#define BSRENDERER_MATERIAL_FONT_H
#include "imaterial.h"
#include "resources/texture.h"
#include "resources/shaders/font_shader.h"
#include "resources/shaders/texture_shader.h"

struct material_font : imaterial {
    texture tex;
    font_shader shader;
    explicit material_font(texture texture, color clr) : tex(std::move(texture)), shader(&tex, clr) {
    }

    ishader *get_shader() override {
        return &shader;
    }
};


#endif //BSRENDERER_MATERIAL_FONT_H