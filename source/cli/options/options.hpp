#ifndef SU_OPTIONS_OPTIONS_HPP
#define SU_OPTIONS_OPTIONS_HPP

#include <string>

namespace options {

struct Options {
    std::string input;

    std::string output;

    float scale = -1.f;
};

Options parse(int argc, char* argv[]) noexcept;

}  // namespace options

#endif
