//
// Created by dawid on 10/30/26.
//

#ifndef BSRENDERER_MATERIAL_H
#define BSRENDERER_MATERIAL_H
#include "renderer/software/shaders/ishader.h"

struct imaterial {
    virtual ~imaterial() = default;

    virtual ishader* get_shader() = 0;
};

#endif //BSRENDERER_MATERIAL_H