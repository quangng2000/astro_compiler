#pragma once

#include "source_location.h"
#include <stdexcept>
#include <string>

class CompilerError : public std::runtime_error {
public:
    CompilerError(const std::string& phase,
                  const std::string& message,
                  SourceLocation loc = {})
        : std::runtime_error(format(phase, message, loc))
        , loc_(loc) {}

    const SourceLocation& location() const { return loc_; }

private:
    SourceLocation loc_;

    static std::string format(const std::string& phase,
                              const std::string& msg,
                              SourceLocation loc) {
        return "[" + phase + "] " + loc.to_string() + ": " + msg;
    }
};
