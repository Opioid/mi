#include "core/model/model_importer.hpp"
#include "core/model/model_exporter_json.hpp"
#include "core/model/model.hpp"

#include <iostream>

int main(int /*argc*/, char* /*argv*/[]) noexcept {
    std::cout << "mi" << std::endl;

    model::Importer importer;

    model::Model* model = importer.read("ImrodLowPoly.obj");

    model::Exporter_json exporter;

    exporter.write("imrod", *model);

    delete model;

    return 0;
}
