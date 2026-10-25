//
// Created by dawid on 10/25/26.
//
#include "renderer/renderer.h"

#include "software/software_renderer.h"
#include "util/log.h"

renderer::renderer(window *window) : _window(window) {
}

renderer * renderer::create_renderer(window *window, renderer_type type) {
    switch (type) {
        case renderer_type::SOFTWARE:
            return new software_renderer(window);

        default:
            log::fatal("renderer::create_renderer: Unknown renderer type -> {}", static_cast<i32>(type));
            return nullptr;
    }
}
