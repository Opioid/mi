#include "model_importer_json.hpp"
#include "base/math/vector3.inl"
#include "model.hpp"
#include "rapidjson/istreamwrapper.h"
#include "triangle_json_handler.hpp"

#include <fstream>

namespace model {

Model* Importer_json::read(std::string const& name) noexcept {
    std::ifstream stream(name, std::ios::binary);
    if (!stream) {
        return nullptr;
    }

    rapidjson::IStreamWrapper json_stream(stream);

    rapidjson::Reader reader;

    Json_handler handler;
    reader.Parse(json_stream, handler);

    if (handler.vertices().empty()) {
        return nullptr;
    }

    if (!handler.has_positions()) {
        return nullptr;
    }

    if (handler.triangles().empty()) {
        return nullptr;
    }

    Model* model = new Model();

    uint32_t const num_parts = handler.parts().size();

    model->allocate_parts(num_parts);

    for (uint32_t i = 0; i < num_parts; ++i) {
        Part const& p = handler.parts()[i];

        Model::Part part{p.num_indices, p.num_indices, p.material_index};

        model->set_part(i, part);
    }

    uint32_t const num_vertices = handler.vertices().size();

    uint32_t const num_indices = handler.triangles().size() * 3;

    model->set_num_vertices(num_vertices);

    model->allocate_positions();

    bool const has_normals = handler.has_normals();

    bool const has_tangents = handler.has_tangents();

    bool const has_uvs = handler.has_texture_coordinates();

    if (has_normals) {
        model->allocate_normals();
    }

    if (has_tangents) {
        model->allocate_tangents();
    }

    if (has_uvs) {
        model->allocate_texture_coordinates();
    }

    for (uint32_t i = 0; i < num_vertices; ++i) {
        auto const& v = handler.vertices()[i];

        model->set_position(i, float3(v.p));

        if (has_tangents && has_normals) {
            model->set_tangent(i, float3(v.t), float3(v.n), v.bitangent_sign > 0 ? -1.f : 1.f);

        } else if (has_normals) {
            model->set_normal(i, float3(v.n));
        }

        if (has_uvs) {
            model->set_texture_coordinate(i, v.uv);
        }
    }

    model->allocate_indices(num_indices);

    for (uint32_t i = 0, len = num_indices / 3; i < len; ++i) {
        auto const& tri = handler.triangles()[i];

        model->set_index(i * 3 + 0, tri.i[0]);
        model->set_index(i * 3 + 1, tri.i[1]);
        model->set_index(i * 3 + 2, tri.i[2]);
    }

    return model;
}

}  // namespace model
