#include "base/math/vector3.inl"
#include "core/model/model.hpp"
#include "core/model/model_exporter_json.hpp"
#include "core/model/model_importer.hpp"

#include <iostream>

int main(int /*argc*/, char* /*argv*/[]) noexcept {
    std::cout << "mi" << std::endl;

    model::Importer importer;

    //   std::string const name = "ImrodLowPoly.obj";
    std::string const name = "scene.gltf";

    model::Model* model = importer.read(name);

    float const scale = 1.f / 12.5f;

    model->scale(float3(scale));

    model::Exporter_json exporter;

    exporter.write("imrodli", *model);

    delete model;

    return 0;
}
