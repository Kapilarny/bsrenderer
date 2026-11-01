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

    const vec3<float>& get_vertex(const i32 face, const i32 vert) const {
        return _vertices[_indices.vertexes[face*3+vert]];
    }

    const vec2<float>& get_uv(const i32 face, const i32 vert) const {
        return _uvs[_indices.uvs[face*3+vert]];
    }

    [[nodiscard]] u64 vertex_count() const { return _vertices.size(); }
    [[nodiscard]] u64 face_count() const { return _indices.vertexes.size() / 3;};
private:
    std::vector<vec3<float>> _vertices;
    std::vector<vec2<float>> _uvs;

    // cache friendliness :)
    struct {
        std::vector<i32> vertexes, uvs, normals;
    } _indices;

    friend class waveform_parser;
    friend class software_renderer; // todo: maybe don't do that?
};

#endif //BSRENDERER_MODEL_H