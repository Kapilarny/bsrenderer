//
// Created by dawid on 10/25/26.
//

#include "model.h"

#include "log.h"
#include "parsers/waveform_parser.h"
#include "platform/platform.h"

model::model(const std::string &model_path) {
    waveform_parser::parse(model_path, this);
}

// std::vector< model::get_vertices() const {
//     return _vertices;
// }
