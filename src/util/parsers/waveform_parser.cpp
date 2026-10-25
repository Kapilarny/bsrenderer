//
// Created by dawid on 10/25/26.
//

#include "waveform_parser.h"

#include <fstream>
#include <ranges>

#include "util/log.h"

bool waveform_parser::parse(const std::string &filename, model* model) {
    std::ifstream ifs(filename.c_str());
    if (!ifs.is_open()) {
        log::error("waveform_parser: Failed to open file -> {}", filename.c_str());
        return false;
    }

    model->_vertices.clear();
    model->_indices.clear();
    std::string command;

    while (ifs >> command) {
        if (command == "v") {
            double x, y, z;
            ifs >> x >> y >> z;
            model->_vertices.emplace_back(x, y, z);
        } else if (command == "f") {
            std::string d;
            ifs >> d;

            // parse
            vec3<i32> indices{};

            // find '/'
            auto f = d.find('/');
            DEBUG_ASSERT(f != std::string::npos, "Invalid waveform file format");
            indices.x = std::stol(d.substr(0, f))-1;

            ifs >> d;
            f = d.find('/');
            DEBUG_ASSERT(f != std::string::npos, "Invalid waveform file format");
            indices.y = std::stol(d.substr(0, f))-1;

            ifs >> d;
            f = d.find('/');
            DEBUG_ASSERT(f != std::string::npos, "Invalid waveform file format");
            indices.z = std::stol(d.substr(0, f))-1;


            model->_indices.emplace_back(indices);

            // TODO: parse it properly
        } else {
            std::string l;
            std::getline(ifs, l); // skip the rest of the line if the command is unrecognized
        }
    }

    return true;
}
