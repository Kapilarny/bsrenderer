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

    auto& input = w.get_input();

    model m("resources/diablo3_pose.obj");
    constexpr auto bg_color = color(0x6095eb);

    renderer* rend = renderer::create_renderer(&w, renderer_type::SOFTWARE);
    while (!w.should_close()) {
        if (input.get_key_state(keyboard_key::A) == key_state::DOWN) {
            m.rotation -= 0.01;
        }
        if (input.get_key_state(keyboard_key::D) == key_state::DOWN) {
            m.rotation += 0.01;
        }

        rend->begin_frame();
        rend->clear_screen(color(0));
        rend->draw_model(m);

        rend->end_frame();
        usleep(1000);
    }

    return 0;
}