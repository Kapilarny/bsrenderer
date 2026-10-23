#include <cstring>
#include <iostream>
#include <cmath>
#include <unistd.h>

#include "util/log.h"

// BSRenderer - Bad Software Renderer

template<typename T>
struct vec2 {
    T x, y;
};

void draw_dot(i32 x, i32 y, framebuffer* buf, u32 color) {
    y = 64-y;

    for (i32 i = 0; i < 16; i++) {
        for (i32 j = 0; j < 16; j++) {
            buf->get_pixels()[((y*16)+i)*64*16+(x*16)+j] = color;
        }
    }
}

void draw_line(vec2<i32> p1, vec2<i32> p2, framebuffer* buf, u32 color) {
    bool steep = std::abs(p1.x - p2.x) < std::abs(p1.y - p2.y);
    if (steep) {
        // if steep transpose
        std::swap(p1.x, p1.y);
        std::swap(p2.x, p2.y);
    }

    if (p1.x > p2.x) {
        std::swap(p1, p2);
    }

    for (i32 x = p1.x; x < p2.x; x++) {
        float t = (x-p1.x) / static_cast<float>(p2.x-p1.x);
        i32 y = std::round(p1.y + t * (p2.y - p1.y));

        if (steep) {
            draw_dot(y, x, buf, color); // detranspose
        } else {
            draw_dot(x, y, buf, color);
        }
    }
}

void rotatePoint(vec2<i32>& p, float angle) {
    constexpr float center = 31.5f;

    float rad = angle * (M_PI / 180.0f);

    float x = p.x - center;
    float y = p.y - center;

    p.x = static_cast<i32>(std::round(
        x * std::cos(rad) - y * std::sin(rad) + center
    ));

    p.y = static_cast<i32>(std::round(
        x * std::sin(rad) + y * std::cos(rad) + center
    ));
}

int main() {
    platform::init();
    log::init();

    auto w = window("bsrenderer", {
        .width = 64*16,
        .height = 64*16,
        .resizable = false
    });

    double hue = 0;

    vec2<i32> p1 = {35, 20};
    vec2<i32> p2 = {22, 37};
    vec2<i32> p3 = {45, 45};

    while (!w.should_close()) {
        const auto buf = w.begin_frame();
        buf->clear_buffer(0xFF000000);

        rotatePoint(p1, 5);
        rotatePoint(p2, 5);
        rotatePoint(p3, 5);

        // hue += 0.001f;

        draw_line(p1, p2, buf, 0xFF0000FF);
        draw_line(p3, p2, buf, 0xFF00FF00);
        draw_line(p1, p3, buf, 0xFFFF0000);

        w.end_frame();
        usleep(50000);
    }

    return 0;
}