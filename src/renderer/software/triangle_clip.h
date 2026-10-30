//
// Created by dawid on 10/28/26.
//

#ifndef BSRENDERER_TRIANGLE_CLIP_H
#define BSRENDERER_TRIANGLE_CLIP_H
#include <vector>

#include "software_renderer.h"
#include "math/vec.h"

namespace triangle_clip {
    constexpr u64 MAX_VERTICES = 12;

    struct polygon {
        std::array<vertex, MAX_VERTICES> vertices{};
        u64 count = 0;

        void push_back(const vertex& v) {
            vertices[count++] = v;
        }
    };

    double plane_distance(const vec4<double>& p, i32 plane);
    vertex intersect(const vertex& a, const vertex& b, i32 plane);
    polygon clip_triangle(const vertex tri[3]);
}


#endif //BSRENDERER_TRIANGLE_CLIP_H