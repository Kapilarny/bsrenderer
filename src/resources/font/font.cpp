//
// Created by dawid on 11/4/26.
//

#include "font.h"

#include <fstream>
#include <sstream>

#include "resources/materials/material_font.h"
#include "resources/materials/material_textured.h"
#include "util/model.h"

model font::generate_text_mesh(const std::string &text) {
    model m{};

    float cursor_x = 0, cursor_y = 0;
    for (char c : text) {
        if (c == '\n') {
            cursor_y -= line_height;
            cursor_x = 0;
            continue;
        }

        if (!_glyphs.contains(c)) {
            log::info("font::generate_text_mesh -> No glyph found for char: {}", c);
            continue;
        }

        const glyph& g = _glyphs[c];

        if (g.width <= 0 || g.height <= 0) { // character empty
            cursor_x += g.x_advance;
            continue;
        }

        const float x0 = cursor_x + g.x_offset;
        const float y0 = cursor_y + base;
        const float x1 = x0 + g.width;
        const float y1 = y0 + g.height;

        const float u0 = static_cast<float>(g.x) / _atlas_width;
        const float v1 = 1.0f - static_cast<float>(g.y) / _atlas_height;
        const float u1 = static_cast<float>(g.x + g.width) / _atlas_width;
        const float v0 = 1.0f - static_cast<float>(g.y + g.height) / _atlas_height;

        const u32 base = m.vertex_count();
        m._vertices.emplace_back(x0, y0, 0.0f);
        m._uvs.emplace_back(u0, v0);

        m._vertices.emplace_back(x1, y0, 0.0f);
        m._uvs.emplace_back(u1, v0);

        m._vertices.emplace_back(x1, y1, 0.0f);
        m._uvs.emplace_back(u1, v1);

        m._vertices.emplace_back(x0, y1, 0.0f);
        m._uvs.emplace_back(u0, v1);

        // First tri
        m._indices.vertexes.emplace_back(base);
        m._indices.uvs.emplace_back(base);
        m._indices.vertexes.emplace_back(base+1);
        m._indices.uvs.emplace_back(base+1);
        m._indices.vertexes.emplace_back(base+2);
        m._indices.uvs.emplace_back(base+2);

        // Second tri
        m._indices.vertexes.emplace_back(base);
        m._indices.uvs.emplace_back(base);
        m._indices.vertexes.emplace_back(base+2);
        m._indices.uvs.emplace_back(base+2);
        m._indices.vertexes.emplace_back(base+3);
        m._indices.uvs.emplace_back(base+3);

        cursor_x += g.x_advance;
    }

    return m;
}

imaterial* font::get_material() const {
    return _material;
}

font::font(const std::string &font_file, const std::string &font_tex) : _material(new material_font(texture(font_tex), color(0xFF0000))) {
    std::ifstream font_desc_file(font_file.c_str());
    if (!font_desc_file.is_open()) {
        log::error("font::font: Failed to open file -> {}", font_file.c_str());
        return;
    }

    // parse font metadata
    _parse_font_meta(font_desc_file);
}

void font::_parse_font_meta(std::ifstream &font_file_stream) {
    auto parse_pair = [](const std::string& pair) -> std::pair<std::string, std::string> {
        std::string a = pair.substr(0, pair.find('='));
        return {a, pair.substr(pair.find('=') + 1)};
    };

    auto retrieve_int = [&parse_pair](std::istringstream& ss) -> i32 {
        std::string pair;
        ss >> pair;
        return std::stol(parse_pair(pair).second);
    };

    std::string line;
    while (std::getline(font_file_stream, line)) {
        std::istringstream ss(line);

        std::string command;
        ss >> command;

        if (command == "info") {
            ss >> command;
            log::info("Loading font: {}", parse_pair(command).second);
        } else if (command == "common") {
            line_height = retrieve_int(ss);
            base = retrieve_int(ss);
            _atlas_width = retrieve_int(ss);
            _atlas_height = retrieve_int(ss);
        } else if (command == "chars") {
            ss >> command;
            auto [key, value] = parse_pair(command);
            FATAL_ASSERT(key == "count", "Invalid font file");

            log::info("font: {} characters found.", value);
        } else if (command == "char") {
            glyph g{};

            g.id = retrieve_int(ss);
            g.x = retrieve_int(ss);
            g.y = retrieve_int(ss);
            g.width = retrieve_int(ss);
            g.height = retrieve_int(ss);
            g.x_offset = retrieve_int(ss);
            g.y_offset = retrieve_int(ss);
            g.x_advance = retrieve_int(ss);

            _glyphs[g.id] = g;
        }
    }
}
