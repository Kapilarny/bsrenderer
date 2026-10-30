//
// Created by dawid on 10/30/26.
//

#ifndef BSRENDERER_TEXTURE_H
#define BSRENDERER_TEXTURE_H
#include "defines.h"
#include "renderer/renderer.h"

#include <vector>

struct texture {
    u32 width{}, height{};
    std::vector<color> pixels;

    texture(const std::string& str);

    color sample(const vec2<double>& uv) const;
};


#endif //BSRENDERER_TEXTURE_H