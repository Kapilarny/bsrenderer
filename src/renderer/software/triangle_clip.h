//
// Created by dawid on 10/28/26.
//

#ifndef BSRENDERER_TRIANGLE_CLIP_H
#define BSRENDERER_TRIANGLE_CLIP_H
#include <vector>

#include "math/vec.h"

namespace triangle_clip {
    constexpr u64 MAX_VERTICES = 12;

    struct polygon {
        std::array<vec4<double>, MAX_VERTICES> vertices{};
        u64 count = 0;

        void push_back(const vec4<double>& v) {
            vertices[count++] = v;
        }
    };

    double plane_distance(const vec4<double>& p, i32 plane);
    vec4<double> intersect(const vec4<double>& a, const vec4<double>& b, i32 plane);
    polygon clip_triangle(const vec4<double> triangle[3]);
}


#endif //BSRENDERER_TRIANGLE_CLIP_H