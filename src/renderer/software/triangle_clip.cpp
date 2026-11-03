//
// Created by dawid on 10/28/26.
//

#include "triangle_clip.h"

#include "software_renderer.h"
#include "tracy/Tracy.hpp"

using polygon = triangle_clip::polygon;

triangle_clip::clip_code triangle_clip::compute_clip_code(const vec4<float> &p) {
    return
        (p.x < -p.w ? 1 << 0 : 0) |
        (p.x >  p.w ? 1 << 1 : 0) |
        (p.y < -p.w ? 1 << 2 : 0) |
        (p.y >  p.w ? 1 << 3 : 0) |
        (p.z < -p.w ? 1 << 4 : 0) |
        (p.z >  p.w ? 1 << 5 : 0);
}

float triangle_clip::plane_distance(const vec4<float> &p, i32 plane) {
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

vertex triangle_clip::intersect(const vertex &a, const vertex &b, i32 plane) {
    const auto da = plane_distance(a.pos, plane);
    const auto db = plane_distance(b.pos, plane);

    const float t = da / (da - db);

    return {
        a.pos + (b.pos - a.pos) * t,
        a.uv + (b.uv - a.uv) * t,
    };
}

polygon triangle_clip::clip_triangle(const triangle tri) {
    // ZoneScoped;

    const auto c0 = compute_clip_code(tri[0].pos);
    const auto c1 = compute_clip_code(tri[1].pos);
    const auto c2 = compute_clip_code(tri[2].pos);

    if ((c0 & c1 & c2) != 0) return {};

    polygon aP, bP;

    for (i32 i = 0; i < 3; i++) {
        aP.push_back(tri[i]);
    }

    if ((c0 | c1 | c2) == 0) {
        return aP;
    }

    polygon *a = &aP, *b = &bP;

    for (i32 plane = 0; plane < 6; plane++) {
        b->count = 0;
        for (i32 i = 0; i < a->count; i++) {
            const auto& curr = a->vertices[i];
            const auto& prev = a->vertices[(i + a->count - 1) % a->count];

            const float curr_dist = plane_distance(curr.pos, plane);
            const float prev_dist = plane_distance(prev.pos, plane);

            const bool curr_inside = curr_dist >= 0.0;
            const bool prev_inside = prev_dist >= 0.0;

            if (curr_inside && prev_inside) {
                b->push_back(curr); // both inside
            } else if (prev_inside && !curr_inside) {
                b->push_back(intersect(prev, curr, plane)); // leaving frustum
            } else if (!prev_inside && curr_inside) {
                b->push_back(intersect(prev, curr, plane)); // entering frustum
                b->push_back(curr);
            }
        }

        std::swap(a, b);

        if (a->count == 0) break;
    }

    return *a;
}
