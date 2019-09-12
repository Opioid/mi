#include "model_exporter_json.hpp"
#include "base/math/vector4.inl"
#include "model.hpp"

#include <fstream>
#include <iostream>

namespace model {

bool Exporter_json::write(const std::string& name, const Model& model) noexcept {
    std::ofstream stream(name + ".json");

    if (!stream) {
        return false;
    }

    stream << "{" << std::endl;

    stream << "\t\"geometry\": {\n";

    // Groups
    stream << "\t\t\"parts\": [\n";

    for (uint32_t i = 0, len = model.num_parts(); i < len; ++i) {
        Model::Part const* parts = model.parts();

        stream << "\t\t\t{" << std::endl;

        stream << "\t\t\t\t\"material_index\": ";
        stream << parts[i].material_index;
        stream << ",\n";

        stream << "\t\t\t\t\"start_index\": ";
        stream << parts[i].start_index;
        stream << ",\n";

        stream << "\t\t\t\t\"num_indices\": ";
        stream << parts[i].num_indices;

        stream << "\n\t\t\t}";

        if (i < len - 1) {
            stream << ",\n";
        }
    }

    stream << "\n\t\t],\n\n";

    // Primitive Topology
    stream << "\t\t\"primitive_topology\": \"triangle_list\",\n\n";

    stream << "\t\t\"vertices\": {\n";

    // Positions

    if (float3 const* positions = model.positions(); positions) {
        stream << "\t\t\t\"positions\": [\n";

        stream << "\t\t\t\t";

        for (uint32_t i = 0, len = model.num_vertices(); i < len; ++i) {
            stream << positions[i][0] << "," << positions[i][1] << "," << positions[i][2];

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t],\n\n";
    }

    // Texture_2D Coordinates
    if (float2 const* texture_coordinates = model.texture_coordinates(); texture_coordinates) {
        stream << "\t\t\t\"texture_coordinates_0\": [\n";

        stream << "\t\t\t\t";

        for (uint32_t i = 0, len = model.num_vertices(); i < len; ++i) {
            stream << texture_coordinates[i][0] << "," << texture_coordinates[i][1];

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t],\n\n";
    }

    // Normals
    if (float3 const* normals = model.normals(); normals) {
        stream << "\t\t\t\"normals\": [\n";

        stream << "\t\t\t\t";

        for (uint32_t i = 0, len = model.num_vertices(); i < len; ++i) {
            stream << normals[i][0] << "," << normals[i][1] << "," << normals[i][2];

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t]";

        if (model.tangents()) {
            stream << ",";
        }

        stream << "\n\n";
    }

    // Tangent Space
    if (float4 const* tangents = model.tangents(); tangents) {
        // Tangents
        stream << "\t\t\t\"tangents_and_bitangent_signs\": [\n";

        stream << "\t\t\t\t";

        for (uint32_t i = 0, len = model.num_vertices(); i < len; ++i) {
            stream << tangents[i][0] << "," << tangents[i][1] << "," << tangents[i][2] << ","
                   << tangents[i][3];

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t]\n\n";
    }

    stream << "\t\t},\n\n";

    // Indices
    stream << "\t\t\"indices\": [\n";

    stream << "\t\t\t";

    uint32_t const* indices = model.indices();

    for (uint32_t i = 0, len = model.num_indices(); i < len; ++i) {
        stream << indices[i];

        if (i < len - 1) {
            stream << ",";
        }

        if ((i + 1) % 8 == 0) {
            stream << "\n\t\t\t";
        }
    }

    stream << "\n\t\t]\n";

    stream << "\t}\n";

    stream << "}";

    return true;
}

}  // namespace model
