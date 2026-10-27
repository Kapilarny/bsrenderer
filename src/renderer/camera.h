//
// Created by dawid on 10/27/26.
//

#ifndef BSRENDERER_CAMERA_H
#define BSRENDERER_CAMERA_H

#include "math/bmath.h"

class camera {
public:
    camera() = default;

    void update();

    void look_at();
    void set_perspective(double f);
    void set_viewport(i32 x, i32 y, i32 w, i32 h);

    vec3<double> pos{}; // TODO: maybe move this to privates?
    vec3<double> up{};
    vec3<double> center{};
private:
    friend class software_renderer;

    mat<4, 4> _viewport;
    mat<4, 4> _perspective;
    mat<4, 4> _model_view;
};

#endif //BSRENDERER_CAMERA_H