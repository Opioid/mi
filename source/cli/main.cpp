#include "base/chrono/chrono.hpp"
#include "base/math/aabb.inl"
#include "base/math/print.hpp"
#include "base/math/vector3.inl"
#include "core/model/model.hpp"
#include "core/model/model_exporter_json.hpp"
#include "core/model/model_exporter_sub.hpp"
#include "core/model/model_importer_assimp.hpp"
#include "core/model/model_importer_json.hpp"
#include "options/options.hpp"

#include <chrono>
#include <iostream>

static std::string autocomplete(std::string const& source, std::string const& addition) noexcept;

static std::string suffix(std::string const&  filename) noexcept;

static std::string extract_filename(std::string const& filename) noexcept;

static std::string discard_extension(std::string const& filename) noexcept;

int main(int argc, char* argv[]) noexcept {
    auto const args = options::parse(argc, argv);

    if (args.input.empty()) {
        std::cout << "No input file specified" << std::endl;

        return 0;
    }

    std::cout << args.input << std::endl;

    for (size_t i = 0, len = args.input.size(); i < len; ++i) {
        std::cout << "=";
    }

    std::cout << std::endl;

    auto const start = std::chrono::high_resolution_clock::now();

    model::Model* model = nullptr;

    if ("json" == suffix(args.input)) {
        model::Importer_json importer;

        model = importer.read(args.input);
    } else {
        model::Importer_assimp importer;

        model = importer.read(args.input);
    }

    if (!model) {
        return 0;
    }

    std::cout << "#triangles: " << model->num_indices() / 3 << std::endl;
    std::cout << "#vertices:  " << model->num_vertices() << std::endl;
    std::cout << "#parts:     " << model->num_parts() << std::endl;
    std::cout << "#materials: " << model->num_materials() << std::endl;

    if (args.scale > 0.f) {
        model->scale(float3(args.scale));
    }

    model->transform(args.transformations);

    model->set_origin(args.origin);

    model->try_to_fix_tangent_space();

    AABB const box = model->aabb();

    std::cout << "AABB: {\n    " << box.bounds[0] << ",\n    " << box.bounds[1] << "}" << std::endl;

    std::string const out = discard_extension(
        args.output.empty() ? args.input : autocomplete(args.output, args.input));

    std::string ext = suffix(args.output);

    if (ext.empty()) {
        ext = "sub";
    }

    model::Exporter_json exporter;

    if ("sub" == ext) {
        model::Exporter_sub exporter_sub;
        exporter_sub.write(out, *model);
    } else if ("json" == ext) {
        exporter.write(out, *model);
    }

    exporter.write_materials(out, extract_filename(out) + "." + ext, *model);

    delete model;

    std::cout << chrono::seconds_since(start) << " s" << std::endl;

    return 0;
}

std::string autocomplete(std::string const& source, std::string const& addition) noexcept {
    if (source[0] == '.') {
        return discard_extension(addition) + source;
    }

    return source;
}

std::string suffix(std::string const&  filename) noexcept {
    size_t const i = filename.find_last_of('.');
    return filename.substr(i + 1, std::string::npos);
}

std::string extract_filename(std::string const& filename) noexcept {
    size_t const i = filename.find_last_of('/') + 1;
    return filename.substr(i, filename.find_first_of('.') - i);
}

std::string discard_extension(std::string const& filename) noexcept {
    return filename.substr(0, filename.find_last_of('.'));
}
