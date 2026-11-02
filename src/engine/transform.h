//
// Created by dawid on 10/31/26.
//

#ifndef BSRENDERER_TRANSFORM_H
#define BSRENDERER_TRANSFORM_H
#include "math/mat.h"
#include "math/vec.h"
#include "util/dirty_value.h"

struct transform {
    vec3<float> position{0, 0, 0};
    vec3<float> rotation{0, 0, 0};
    vec3<float> scale{1, 1, 1};

    void mark_dirty() {
        _dirty = true;
    }

    bool should_rebuild() {
        return _dirty;
    }

    mat<4, 4> rebuild_matrix() {
        const auto& pos = position;
        const auto& rot = rotation;
        const auto& sc = scale;

        const auto translation = mat<4, 4>{
            1, 0, 0, pos.x,
            0, 1, 0, pos.y,
            0, 0, 1, pos.z,
            0,0, 0, 1
        };

        const float cx = std::cos(rot.x);
        const float cy = std::cos(rot.y);
        const float cz = std::cos(rot.z);

        const float sx = std::sin(rot.x);
        const float sy = std::sin(rot.y);
        const float sz = std::sin(rot.z);

        const auto rot_x = mat<4, 4>{
            1, 0, 0, 0,
            0, cx, -sx, 0,
            0, sx, cx, 0,
            0, 0, 0, 1
        };

        const auto rot_y = mat<4, 4>{
            cy, 0, sy, 0,
            0, 1, 0, 0,
            -sy, 0, cy, 0,
            0, 0, 0, 1
        };

        const auto rot_z = mat<4, 4>{
            cz, -sz, 0, 0,
            sz, cz, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };

        const auto scaling = mat<4, 4>{
            sc.x, 0, 0, 0,
            0, sc.y, 0, 0,
            0, 0, sc.z, 0,
            0, 0, 0, 1
        };

        _dirty = false;

        return translation * rot_z * rot_y * rot_x * scaling;
    }

private:
    bool _dirty = true;
};

#endif //BSRENDERER_TRANSFORM_H