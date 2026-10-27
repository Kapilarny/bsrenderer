//
// Created by dawid on 10/27/26.
//

#include "camera.h"

void camera::update() {
    look_at();
}

void camera::look_at() {
    const vec3 n = (pos-center).normalized();
    const vec3 l = bmath::cross(up, n).normalized();
    const vec3 m = bmath::cross(n, l).normalized();

    _model_view = mat<4, 4>{
        {
            l.x, l.y, l.z, 0,
            m.x, m.y, m.z, 0,
            n.x, n.y, n.z, 0,
            0, 0, 0, 1
        }}
    * mat<4, 4>{
        {
            1, 0, 0, -center.x,
            0, 1, 0, -center.y,
            0, 0, 1, -center.z,
            0, 0, 0, 1
        }
    };
}

void camera::set_perspective(const double f) {
    _perspective = {
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, -1.0/f, 1
        }
    };
}

void camera::set_viewport(i32 x, i32 y, i32 w, i32 h) {
    _viewport = {
        {
            w/2.0, 0, 0, x+w/2.0,
            0, h/2.0, 0, y+h/2.0,
            0, 0, 1, 0,
            0, 0, 0, 1
        }
    };
}
