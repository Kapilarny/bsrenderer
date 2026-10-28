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

    void move(double forward, double right, double up, double dt);
    void rotate(double dx, double dy);

    void set_perspective(double fovy, double aspect, double near, double far);
    void set_viewport(i32 x, i32 y, i32 w, i32 h);

    vec3<double> pos{}; // TODO: maybe move this to privates?
    vec3<double> _up{0, 1, 0}, right{};

    double yaw = -std::numbers::pi / 2.0;
    double pitch{};
private:
    friend class software_renderer;

    void _look_at();
    vec3<double> _forward() const;
    vec3<double> _right() const;

    mat<4, 4> _viewport;
    mat<4, 4> _projection;
    mat<4, 4> _view;
};

#endif //BSRENDERER_CAMERA_H