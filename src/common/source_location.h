#pragma once

#include <string>

struct SourceLocation {
    int line = 1;
    int col = 1;

    std::string to_string() const {
        return std::to_string(line) + ":" + std::to_string(col);
    }
};
