#include "options.hpp"

#include <assimp/version.h>
#include <cctype>
#include <iostream>

namespace options {

static bool handle_all(std::string const&, std::string const& parameter, Options& result) noexcept;

static bool handle(std::string const&, std::string const& parameter, Options& result) noexcept;

static bool is_parameter(std::string_view text) noexcept;

static void help() noexcept;

Options parse(int argc, char* argv[]) noexcept {
    Options result;

    result.transformations.clear();

    if (1 == argc) {
        help();
        return result;
    }

    for (int32_t i = 1; i < argc;) {
        std::string const command = std::string(argv[i]).substr(1);

        int32_t j = i + 1;
        for (;; ++j) {
            if (j < argc && is_parameter(argv[j])) {
                handle_all(command, argv[j], result);
            } else {
                if (j == i + 1) {
                    handle_all(command, "", result);
                }

                break;
            }
        }

        i = j;
    }

    return result;
}

bool handle_all(std::string const& command, std::string const& parameter,
                Options& result) noexcept {
    if (command[0] == '-') {
        return handle(command.substr(1), parameter, result);
    }

    for (size_t i = 0, len = command.size(); i < len; ++i) {
        if (!handle(command.substr(i, 1), parameter, result)) {
            return false;
        }
    }

    return true;
}

bool handle(std::string const& command, std::string const& parameter, Options& result) noexcept {
    using namespace model;

    if ("help" == command || "h" == command) {
        help();
    } else if ("in" == command || "i" == command) {
        result.input = parameter;
    } else if ("out" == command || "o" == command) {
        result.output = parameter;
    } else if ("center-bottom" == command) {
        result.origin = Model::Origin::Center_bottom;
    } else if ("reverse-x" == command) {
        result.transformations.set(Model::Transformation::Reverse_X);
    } else if ("reverse-y" == command) {
        result.transformations.set(Model::Transformation::Reverse_Y);
    } else if ("reverse-z" == command) {
        result.transformations.set(Model::Transformation::Reverse_Z);
    } else if ("reverse-xz" == command || "reverse-zx" == command) {
        result.transformations.set(Model::Transformation::Reverse_X);
        result.transformations.set(Model::Transformation::Reverse_Z);
    } else if ("reverse-yz" == command || "reverse-zx" == command) {
        result.transformations.set(Model::Transformation::Reverse_Y);
        result.transformations.set(Model::Transformation::Reverse_Z);
    } else if ("scale" == command || "s" == command) {
        result.scale = float(std::atof(parameter.data()));
    } else if ("swap-xy" == command || "swap-yx" == command) {
        result.transformations.set(Model::Transformation::Swap_XY);
    } else if ("swap-yz" == command || "swap-zy" == command) {
        result.transformations.set(Model::Transformation::Swap_YZ);
    } else {
        std::cout << "Option " << command << " does not exist.";
    }

    return true;
}

bool is_parameter(std::string_view text) noexcept {
    if (text.size() <= 1) {
        return true;
    }

    if (text[0] == '-') {
        if (text[1] == '-') {
            return false;
        }

        for (size_t i = 1, len = text.size(); i < len; ++i) {
            if (!std::isdigit(text[i])) {
                return false;
            }
        }
    }

    return true;
}

void help() noexcept {
    static std::string const usage =
        R"(mi is a model importer
Usage:
  it [OPTION...]

  -h, --help           Print help.
  -i, --in     file    File name of the input model.
  -o, --out    file    File name of the output files, without extension.
      --center-bottom  Set the model's origin to the center bottom,
                       e.g. [0, -1, 0] for the unit cube.
      --reverse-[xzz]  Reverse the specified axis of the model's vertices.
  -s, --scale  float   Scalar (> 0) to uniformly scale the model by.)";

    std::cout << usage << "\n\n";

    std::cout << "Dependencies:\n";

    std::cout << "  Assimp " << aiGetVersionMajor() << "." << aiGetVersionMinor() << "."
              << aiGetVersionRevision() << std::endl;
}

}  // namespace options
