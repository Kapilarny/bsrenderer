//
// Created by dawid on 10/23/26.
//

#include "platform/platform.h"

void framebuffer::clear_buffer(const u32 background) const {
    std::fill_n(
        get_pixels(),
        get_width() * get_height(),
        background
    );
}