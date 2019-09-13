#include "base/math/aabb.inl"
#include "base/math/print.hpp"
#include "base/math/vector3.inl"
#include "core/model/model.hpp"
#include "core/model/model_exporter_json.hpp"
#include "core/model/model_importer.hpp"
#include "options/options.hpp"

#include <iostream>

std::string discard_extension(std::string const& filename) noexcept;

int main(int argc, char* argv[]) noexcept {
    auto const args = options::parse(argc, argv);

    if (args.input.empty()) {
        std::cout << "No input file specified" << std::endl;

        return 0;
    }

    model::Importer importer;

    model::Model* model = importer.read(args.input);

    if (!model) {
        return 0;
    }

    if (args.scale > 0.f) {
        model->scale(float3(args.scale));
    }

    if (!args.transformations.empty()) {
        model->transform(args.transformations);
    }

    std::cout << "\"" << args.input << "\"" << std::endl;

    for (size_t i = 0, len = args.input.size() + 2; i < len; ++i) {
        std::cout << "=";
    }

    std::cout << std::endl;

    std::cout << "#triangles: " << model->num_indices() / 3 << std::endl;
    std::cout << "#vertices:  " << model->num_vertices() << std::endl;
    std::cout << "#parts:     " << model->num_parts() << std::endl;
    std::cout << "#materials: " << model->num_materials() << std::endl;

    AABB const box = model->aabb();

    std::cout << "AABB: {\n    " << box.bounds[0] << ",\n    " << box.bounds[1] << "}" << std::endl;

    model::Exporter_json exporter;

    std::string const out = args.output.empty() ? discard_extension(args.input) : args.output;

    exporter.write(out, *model);
    exporter.write_materials(out, *model);

    delete model;

    return 0;
}

std::string discard_extension(std::string const& filename) noexcept {
    return filename.substr(0, filename.find_first_of('.'));
}
