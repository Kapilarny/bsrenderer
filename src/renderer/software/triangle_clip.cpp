//
// Created by dawid on 10/28/26.
//

#include "triangle_clip.h"

using polygon = triangle_clip::polygon;

double triangle_clip::plane_distance(const vec4<double> &p, i32 plane) {
    switch (plane) {
        case 0: return p.w + p.x; // left
        case 1: return p.w - p.x; // right
        case 2: return p.w + p.y; // bottom
        case 3: return p.w - p.y; // top
        case 4: return p.w + p.z; // near
        case 5: return p.w - p.z; // far

        default: break;
    }

    return -1.0;
}

vec4<double> triangle_clip::intersect(const vec4<double> &a, const vec4<double> &b, i32 plane) {
    const auto da = plane_distance(a, plane);
    const auto db = plane_distance(b, plane);

    const double t = da / (da - db);

    return a + (b - a) * t;
}

polygon triangle_clip::clip_triangle(const vec4<double> triangle[3]) {
    polygon a, b;

    for (i32 i = 0; i < 3; i++) {
        a.push_back(triangle[i]);
    }

    for (i32 plane = 0; plane < 6; plane++) {
        bool all_outside = true;
        for (i32 i = 0; i < a.count; i++) {
            if (plane_distance(a.vertices[i], plane) >= 0.0) {
                all_outside = false;
                break;
            }
        }

        // quick escape if all verts are outside the plane
        if (all_outside) {
            a.count = 0;
            break;
        }

        b.count = 0;
        for (i32 i = 0; i < a.count; i++) {
            const auto& curr = a.vertices[i];
            const auto& prev = a.vertices[(i + a.count - 1) % a.count];

            const double curr_dist = plane_distance(curr, plane);
            const double prev_dist = plane_distance(prev, plane);

            const bool curr_inside = curr_dist >= 0.0;
            const bool prev_inside = prev_dist >= 0.0;

            if (curr_inside && prev_inside) {
                b.push_back(curr); // both inside
            } else if (prev_inside && !curr_inside) {
                b.push_back(intersect(prev, curr, plane)); // leaving frustum
            } else if (!prev_inside && curr_inside) {
                b.push_back(intersect(prev, curr, plane)); // entering frustum
                b.push_back(curr);
            }
        }

        std::swap(a, b);

        if (a.count == 0) break;
    }

    return a;
}
