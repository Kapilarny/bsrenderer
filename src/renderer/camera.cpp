//
// Created by dawid on 10/27/26.
//

#include "camera.h"

void camera::update() {
    _look_at();
}

void camera::move(double forward, double right, double up, double dt) {
    const auto f = _forward();
    const auto r = _right();
    const auto u = _up;

    vec3<double> m = f * forward + r * right + u * up;

    // This fixes diagonal movement being faster
    if (m.magnitude() > 0.0) {
        m = m.normalized();
    }

    constexpr double speed = 5.0;
    pos += m * speed * dt;
}

void camera::rotate(double dx, double dy) {
    constexpr double sensitivity = 0.002;

    yaw += sensitivity * dx;
    pitch -= sensitivity * dy;

    constexpr double limit = std::numbers::pi / 2.0 - 0.01;
    pitch = std::clamp(pitch, -limit, limit);
}

void camera::_look_at() {
    const auto f = _forward().normalized();
    const auto r = _right();
    const auto u = bmath::cross(r, f).normalized();

    _view = {{
        r.x, r.y, r.z, -bmath::dot<double>(r, pos),
        u.x, u.y, u.z, -bmath::dot<double>(u, pos),
        -f.x, -f.y, -f.z, bmath::dot<double>(f, pos),
        0, 0, 0, 1
    }};
}

vec3<double> camera::_forward() const {
    return {
        std::cos(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::sin(yaw) * std::cos(pitch)
    };
}

vec3<double> camera::_right() const {
    return bmath::cross(_forward(), _up).normalized();
}

void camera::set_perspective(double fovy, double aspect, double near, double far) {
    const auto t = 1.0 / std::tan(fovy / 2.0);

    _projection = {{
        t/aspect, 0, 0, 0,
        0, t, 0, 0,
        0, 0,
        -(far + near) / (far - near),
        -(2.0 * far * near) / (far - near),
        0, 0, -1, 0
    }};
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
