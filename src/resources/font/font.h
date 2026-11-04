//
// Created by dawid on 11/4/26.
//

#ifndef BSRENDERER_FONT_H
#define BSRENDERER_FONT_H
#include <unordered_map>

#include "defines.h"
#include "resources/texture.h"

struct glyph {
    u32 id, x, y, width, height;
    i32 x_offset, y_offset, x_advance;
};

class font {
public:
    u32 line_height{};
    u32 base{};

    explicit font(const std::string& font_file, const std::string& font_tex);

    model generate_text_mesh(const std::string& text);
    imaterial* get_material() const;
private:
    void _parse_font_meta(std::ifstream &font_file_stream);

    std::unordered_map<u32, glyph> _glyphs;
    u32 _atlas_width, _atlas_height;
    imaterial* _material;
};

#endif //BSRENDERER_FONT_H