//
// Created by dawid on 10/25/26.
//

#ifndef BSRENDERER_MODEL_H
#define BSRENDERER_MODEL_H

#include <string>
#include <vector>

#include "defines.h"

#include "math/vec.h"

class framebuffer;

struct face_index {
    i32 vertex;
    i32 uv;
    i32 normal;
};

class model {
public:
    explicit model(const std::string& model_path);

    double rotation = 0; // TODO: remove this lol
    std::vector<vec3<double>> vertices;
    std::vector<vec3<face_index>> indices;
    std::vector<vec2<double>> uvs;
private:
    friend class waveform_parser;
    friend class software_renderer; // todo: maybe don't do that?
};

#endif //BSRENDERER_MODEL_H