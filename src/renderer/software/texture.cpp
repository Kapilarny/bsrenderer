//
// Created by dawid on 10/30/26.
//

#include "texture.h"

#include "external/stb_image.h"

texture::texture(const std::string &str) {
    i32 w, h, channels;

    // TODO: flip this if shit doesnt work
    stbi_set_flip_vertically_on_load(true);

    const auto data = stbi_load(str.c_str(), &w, &h, &channels, 4);
    if (!data) {
        log::error("failed to load texture %s", str.c_str());
        return;
    }

    width = w;
    height = h;

    pixels.resize(w*h);
    for (u32 i = 0; i < w * h; i++) {
        const u8 r = data[i*4];
        const u8 g = data[i*4+1];
        const u8 b = data[i*4+2];
        const u8 a = data[i*4+3];

        pixels[i] = color(r, g, b, a);
    }

    stbi_image_free(data);
}

color texture::sample(const vec2<double> &uv) const {
    const double u = std::clamp(uv.x, 0.0, 1.0);
    const double v = std::clamp(uv.y, 0.0, 1.0);

    const u32 x = static_cast<u32>(u * (width-1));
    const u32 y = static_cast<u32>(v * (height-1));

    return pixels[y*width + x];
}
