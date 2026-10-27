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

    constexpr i32 width = 1024;
    constexpr i32 height = 1024;

    auto w = window("bsrenderer", {
        .width = width,
        .height = height,
        .resizable = false
    });

    const auto& input = w.get_input();

    model m("resources/diablo3_pose.obj");
    constexpr auto bg_color = color(0x6095eb);

    const auto rend = renderer::create_renderer(&w, renderer_type::SOFTWARE);

    auto cam = camera();
    cam.pos = {0, 0, 3};
    cam.center = {0, 0, 0};
    cam.up = {0, 1, 0};

    cam.look_at();
    cam.set_perspective((cam.pos-cam.center).magnitude());
    cam.set_viewport(width/16, height/16, width*7/8, height*7/8);

    rend->attach_camera(&cam);

    while (!w.should_close()) {
        if (input.get_key_state(keyboard_key::ESCAPE) == key_state::PRESSED) {
            log::info("Quitting...");
            break;
        }

        if (input.get_key_state(keyboard_key::A) == key_state::DOWN) {
            cam.pos.x -= 0.01;
        }
        if (input.get_key_state(keyboard_key::D) == key_state::DOWN) {
            cam.pos.x += 0.01;
        }

        cam.update();

        rend->begin_frame();

        rend->clear_screen(color(0));
        rend->draw_model(m);
        rend->end_frame();

        usleep(1000);
    }

    return 0;
}