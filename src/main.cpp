#include <chrono>
#include <cstring>
#include <iostream>
#include <cmath>
#include <thread>
#include <unistd.h>

#include "renderer/renderer.h"
#include "renderer/software/materials/material_textured.h"
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
    material_textured tex_material(texture("resources/diablo3_pose_diffuse.tga"));

    constexpr auto bg_color = color(0x6095eb);
    constexpr auto txt_color = color(0xff0000);

    const auto rend = renderer::create_renderer(&w, renderer_type::SOFTWARE);

    auto cam = camera();
    cam.pos = {0.0, 0.0, 2};

    cam.set_perspective(60.0 * std::numbers::pi / 180.0, static_cast<double>(width) / static_cast<double>(height), 0.1, 1000);
    cam.set_viewport(0, 0, width, height);

    w.capture_mouse();
    rend->attach_camera(&cam);


    constexpr double TARGET_FPS = 165.0;
    constexpr auto target_frame_time = std::chrono::duration<double>(1.0/TARGET_FPS);
    auto last_frame = std::chrono::steady_clock::now();
    u64 last_frame_ms = 0;

    while (!w.should_close()) {
        const auto frame_start = std::chrono::steady_clock::now();
        const double delta_time = std::chrono::duration<double>(frame_start-last_frame).count();
        last_frame = frame_start;

        if (input.get_key_state(keyboard_key::ESCAPE) == key_state::PRESSED) {
            log::info("Quitting...");
            break;
        }

        double forward = input.is_key_down(keyboard_key::W) - input.is_key_down(keyboard_key::S);
        double right = input.is_key_down(keyboard_key::D) - input.is_key_down(keyboard_key::A);
        cam.move(forward, right, 0, delta_time);

        auto [delta_x, delta_y] = input.get_mouse_delta();
        cam.rotate(delta_x, delta_y);
        cam.update();

        rend->begin_frame();

        rend->clear_screen(color(0));
        rend->draw_model(m, tex_material);

        rend->blit_text("bsrenderer", 10, 80, txt_color);
        rend->blit_text("frametime: " + std::to_string(last_frame_ms), 10, 10, txt_color);
        rend->end_frame();

        const auto elapsed = std::chrono::steady_clock::now() - frame_start;
        const auto frame_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        last_frame_ms = frame_time_ms;

        if (elapsed < target_frame_time) {
            std::this_thread::sleep_for(target_frame_time - elapsed);
        }
    }

    return 0;
}