#ifndef SU_OPTIONS_OPTIONS_HPP
#define SU_OPTIONS_OPTIONS_HPP

#include "base/flags/flags.hpp"
#include "core/model/model.hpp"

#include <string>

namespace options {

struct Options {
    std::string input;

    std::string output;

    model::Model::Origin origin = model::Model::Origin::Default;

    float scale = -1.f;

    flags::Flags<model::Model::Transformation> transformations;
};

Options parse(int argc, char* argv[]) noexcept;

}  // namespace options

#endif
