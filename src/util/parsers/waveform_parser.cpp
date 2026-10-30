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

    model->vertices.clear();
    model->indices.clear();

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
            model->vertices.emplace_back(x, y, z);
        } else if (command == "f") {
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
            vec3<face_index> idx;
            for (int i = 0; i < 3; i++) {
                if (!(iss >> part)) {
                    log::error("Bad waveform file!");
                    return false;
                }

                idx[i] = read_face_idx(part);
            }

            model->indices.emplace_back(idx);
            // TODO: parse it properly
        } else if (command == "vt") {
            vec2<double> uv;
            iss >> uv.x;
            iss >> uv.y;
            model->uvs.emplace_back(uv);
        }
    }

    return true;
}
