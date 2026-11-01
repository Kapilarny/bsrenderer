//
// Created by dawid on 10/25/26.
//

#include "waveform_parser.h"

#include <fstream>
#include <ranges>
#include <sstream>

#include "util/log.h"

bool waveform_parser::parse(const std::string &filename, model* model) {
    std::ifstream ifs(filename.c_str());
    if (!ifs.is_open()) {
        log::error("waveform_parser: Failed to open file -> {}", filename.c_str());
        return false;
    }

    model->_vertices.clear();
    model->_indices.vertexes.clear();
    model->_indices.normals.clear();
    model->_indices.uvs.clear();

    // std::string command;
    std::string line;

    while (!ifs.eof()) {
        std::getline(ifs, line);
        std::istringstream iss(line);

        std::string command;
        iss >> command;

        if (command == "v") {
            double x, y, z;
            iss >> x >> y >> z;
            model->_vertices.emplace_back(x, y, z);
        } else if (command == "f") {
            // this is the laziest shit of all time, but idc
            struct face_index {
                i32 vertex;
                i32 uv;
                i32 normal;
            };

            auto read_face_idx = [](std::string& part) {
                face_index idx{-1, -1, -1};

                std::istringstream ts(part);
                if (std::getline(ts, part, '/')) {
                    idx.vertex = std::stoi(part) - 1;
                }

                if (std::getline(ts, part, '/') && !part.empty()) {
                    idx.uv = std::stoi(part) - 1;
                }

                if (std::getline(ts, part, '/') && !part.empty()) {
                    idx.normal = std::stoi(part) - 1;
                }

                return idx;
            };

            std::string part;
            // vec3<face_index> idx;
            for (int i = 0; i < 3; i++) {
                if (!(iss >> part)) {
                    log::error("Bad waveform file!");
                    return false;
                }

                auto f = read_face_idx(part);
                model->_indices.vertexes.emplace_back(f.vertex);
                model->_indices.normals.emplace_back(f.normal);
                model->_indices.uvs.emplace_back(f.uv);
            }

            // model->indices.emplace_back(idx);
            // TODO: parse it properly
        } else if (command == "vt") {
            vec2<float> uv;
            iss >> uv.x;
            iss >> uv.y;
            model->_uvs.emplace_back(uv);
        }
    }

    return true;
}
