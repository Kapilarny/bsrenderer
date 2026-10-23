#include <cstring>
#include <iostream>
#include <cmath>

#include "util/log.h"

// BSRenderer - Bad Software Renderer
uint32_t hsvToArgb(double h, double s, double v, const u8 a = 255) {
    h = std::fmod(h, 360.0);
    if (h < 0.0) h += 360.0;

    s = std::clamp(s, 0.0, 1.0);
    v = std::clamp(v, 0.0, 1.0);

    double c = v * s;
    double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    double m = v - c;

    double r = 0, g = 0, b = 0;

    if      (h < 60)  { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else              { r = c; b = x; }

    auto toU8 = [](double n) -> uint32_t {
        return static_cast<uint32_t>(std::round((n) * 255.0));
    };

    uint32_t R = toU8(r + m);
    uint32_t G = toU8(g + m);
    uint32_t B = toU8(b + m);

    return (static_cast<uint32_t>(a) << 24)
         | (R << 16)
         | (G << 8)
         | B;
}


int main() {
    platform::init();
    log::init();

    auto w = window("bsrenderer", {
        .width = 640,
        .height = 480,
        .resizable = false
    });

    double hue = 0;
    while (!w.should_close()) {
        const auto buf = w.begin_frame();
        hue += 0.01;
        buf->clear_buffer(hsvToArgb(hue, 1, 1));

        w.end_frame();
    }

    return 0;
}