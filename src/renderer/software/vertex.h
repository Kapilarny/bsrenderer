//
// Created by dawid on 10/30/26.
//

#ifndef BSRENDERER_VERTEX_H
#define BSRENDERER_VERTEX_H
#include "math/vec.h"

struct vertex {
    vec4<double> pos;
    vec2<double> uv;
};

using triangle = vertex[3];

#endif //BSRENDERER_VERTEX_H