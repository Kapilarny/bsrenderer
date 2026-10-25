#include <cstring>
#include <iostream>
#include <cmath>
#include <unistd.h>

#include "renderer/renderer.h"
#include "util/log.h"
#include "util/model.h"

// BSRenderer - Bad Software Renderer

int main() {
    platform::init();
    log::init();

    auto w = window("bsrenderer", {
        .width = 1024,
        .height = 1024,
        .resizable = false
    });

    model m("resources/diablo3_pose.obj");

    constexpr auto bg_color = color(0x6095eb);

    renderer* rend = renderer::create_renderer(&w, renderer_type::SOFTWARE);
    while (!w.should_close()) {
        rend->begin_frame();

        rend->clear_screen(color(0));
        rend->draw_model(m);

        rend->end_frame();
        usleep(50000);
    }

    return 0;
}