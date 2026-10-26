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

class model {
public:
    explicit model(const std::string& model_path);

    double rotation = 0; // TODO: remove this lol
private:
    friend class waveform_parser;
    friend class software_renderer; // todo: maybe don't do that?
    std::vector<vec3<double>> _vertices;
    std::vector<vec3<i32>> _indices;
};

#endif //BSRENDERER_MODEL_H