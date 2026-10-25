//
// Created by dawid on 10/25/26.
//

#ifndef BSRENDERER_WAVEFORM_PARSER_H
#define BSRENDERER_WAVEFORM_PARSER_H
#include <string>
#include <vector>

#include "defines.h"
#include "util/model.h"

class waveform_parser {
public:
    static bool parse(const std::string& filename, model* model);
};

#endif //BSRENDERER_WAVEFORM_PARSER_H