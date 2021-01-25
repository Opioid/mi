#include "model_exporter_json.hpp"
#include "base/math/print.hpp"
#include "base/math/vector4.inl"
#include "model.hpp"

#include "rapidjson/filewritestream.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"

#include <fstream>
#include <iomanip>

namespace model {

bool Exporter_json::write(std::string const& name, Model const& model) const noexcept {
    std::ofstream stream(name + ".json");

    if (!stream) {
        return false;
    }

    /*
    rapidjson::OStreamWrapper json_stream(stream);

    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(json_stream);

    writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
    writer.SetMaxDecimalPlaces(6);

    writer.StartObject();

    writer.Key("geometry");
    writer.StartObject();

    // Parts
    writer.Key("parts");
    writer.StartArray();

    Model::Part const* parts = model.parts();
    for (uint32_t i = 0, len = model.num_parts(); i < len; ++i) {
        writer.StartObject();

        writer.Key("material_index");
        writer.Uint(parts[i].material_index);

        writer.Key("start_index");
        writer.Uint(parts[i].start_index);

        writer.Key("num_indices");
        writer.Uint(parts[i].num_indices);

        writer.EndObject();
    }

    writer.EndArray();

    // Primitive Topology
    writer.Key("primitive_topology");
    writer.String("triangle_list");

    // Vertices
    {
    writer.Key("vertices");
    writer.StartObject();

    // Positions
    if (float3 const* positions = model.positions(); positions) {
        writer.Key("positions");
        writer.StartArray();

        for (uint32_t i = 0, len = model.num_vertices(); i < len; ++i) {
            writer.Double(positions[i][0]);
            writer.Double(positions[i][1]);
            writer.Double(positions[i][2]);
        }

        writer.EndArray();
    }

    // Texture_2D Coordinates
    if (float2 const* uvs = model.texture_coordinates(); uvs) {
        writer.Key("texture_coordinates_0");
        writer.StartArray();

        for (uint32_t i = 0, len = model.num_vertices(); i < len; ++i) {
            writer.Double(uvs[i][0]);
            writer.Double(uvs[i][1]);
        }

        writer.EndArray();
    }

    writer.EndObject();
    }

    writer.EndObject();

    writer.EndObject();
    */

    stream << std::setprecision(7);

    stream << "{\n";

    stream << "\t\"geometry\": {\n";

    // Parts
    stream << "\t\t\"parts\": [\n";

    Model::Part const* parts = model.parts();
    for (uint32_t i = 0, len = model.num_parts(); i < len; ++i) {
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

    static bool constexpr tangent_space_as_quaternion = true;

    float3 const* normals  = model.normals();
    float4 const* tangents = model.tangents();

    if (tangent_space_as_quaternion && normals && tangents) {
        // Tangent space
        stream << "\t\t\t\"tangent_space\": [\n";

        stream << "\t\t\t\t";

        for (uint32_t i = 0, len = model.num_vertices(); i < len; ++i) {
            float4 const t = tangents[i];
            float3 const n = normals[i];

            Quaternion const ts = Model::tangent_space(t.xyz(), n, t[3]);

            stream << ts[0] << "," << ts[1] << "," << ts[2] << "," << ts[3];

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t]\n\n";
    } else {
        // Normals
        if (normals) {
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

        // Tangents
        if (tangents) {
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

template <class Writer>
static void put_texture(Writer& writer, std::string_view usage, std::string const& name) {
    if (name.empty()) {
        return;
    }

    writer.StartObject();

    writer.Key("usage");
    writer.String(usage.data());
    writer.Key("file");
    writer.String(name.c_str());

    writer.EndObject();
}

bool Exporter_json::write_materials(std::string const& name, Model const& model) const noexcept {
    auto const* materials = model.materials();

    if (!materials) {
        return true;
    }

    std::ofstream stream(name + ".scene");

    if (!stream) {
        return false;
    }


    rapidjson::OStreamWrapper json_stream(stream);

    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(json_stream);

    writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
    writer.SetMaxDecimalPlaces(5);

    writer.StartObject();

    writer.Key("materials");
    writer.StartArray();

    for (uint32_t i = 0, len = model.num_materials(); i < len; ++i) {
        auto const& m = materials[i];

        writer.StartObject();
        writer.Key("name");
        writer.String(m.name.c_str());

        writer.Key("rendering");
        writer.StartObject();

        writer.Key("Substitute");
        writer.StartObject();

        bool const has_textures = !m.mask_texture.empty() || !m.color_texture.empty() ||
                                  !m.normal_texture.empty();

        if (has_textures) {
            writer.Key("textures");
            writer.StartArray();

            put_texture(writer, "Mask", m.mask_texture);
            put_texture(writer, "Color", m.color_texture);
            put_texture(writer, "Normal", m.normal_texture);

            if (!m.roughness_texture.empty()) {
                put_texture(writer, "Roughness", m.roughness_texture);
            } else {
                put_texture(writer, "Shininess", m.shininess_texture);
            }

            writer.EndArray();
        }

        if (m.color_texture.empty()) {
            writer.Key("color");
            writer.StartArray();
            writer.Double(m.diffuse_color[0]);
            writer.Double(m.diffuse_color[1]);
            writer.Double(m.diffuse_color[2]);
            writer.EndArray();
        }

        writer.Key("roughness");
        writer.Double(m.roughness);

        if (m.two_sided) {
            writer.Key("two_sided");
            writer.Bool(m.two_sided);
        }

        writer.EndObject();
        writer.EndObject();
        writer.EndObject();
    }


    writer.EndArray();

    writer.Key("entities");
    writer.StartArray();

    writer.StartObject();

    writer.Key("type");
    writer.String("Prop");
    writer.Key("shape");
    writer.StartObject();
    writer.Key("file");
    writer.String(name.c_str());
    writer.EndObject();

    writer.SetFormatOptions(rapidjson::kFormatDefault);

    writer.Key("materials");
    writer.StartArray();
    for (uint32_t i = 0, len = model.num_materials(); i < len; ++i) {
        writer.String(materials[i].name.c_str());
    }
    writer.EndArray();

    writer.EndObject();

    writer.EndArray();

    writer.EndObject();



    /*
    stream << "{\n";

    stream << "\t\"materials\": [\n\t\t";

    for (uint32_t i = 0, len = model.num_materials(); i < len; ++i) {
        auto const& m = materials[i];

        stream << "{\n";
        stream << "\t\t\t\"name\": \"" << m.name << "\",\n";

        stream << "\t\t\t\"rendering\": {\n";

        stream << "\t\t\t\t\"Substitute\": {\n";

        bool const has_textures = !m.mask_texture.empty() || !m.color_texture.empty() ||
                                  !m.normal_texture.empty();

        bool previous = false;

        if (has_textures) {
            stream << "\t\t\t\t\t\"textures\": [\n";

            put_texture(stream, "Mask", m.mask_texture, previous);
            put_texture(stream, "Color", m.color_texture, previous);
            put_texture(stream, "Normal", m.normal_texture, previous);

            if (!m.roughness_texture.empty()) {
                put_texture(stream, "Roughness", m.roughness_texture, previous);
            } else {
                put_texture(stream, "Shininess", m.shininess_texture, previous);
            }

            stream << "\n\t\t\t\t\t],\n";
        }

        if (m.color_texture.empty()) {
            stream << "\t\t\t\t\t\"color\": " << m.diffuse_color << ",\n";
        }

        stream << "\t\t\t\t\t\"roughness\": " << m.roughness;

        if (m.two_sided) {
            stream << ",\n\t\t\t\t\t\"two_sided\": true\n";
        } else {
            stream << "\n";
        }

        stream << "\t\t\t\t}\n";

        stream << "\t\t\t}\n";

        stream << "\t\t}";

        if (i < len - 1) {
            stream << ", ";
        }
    }

    stream << "\n\t],\n\n";

    stream << "\t\"names\": [\n";

    for (uint32_t i = 0, len = model.num_materials(); i < len; ++i) {
        auto const& m = materials[i];

        stream << "\t\t\"" << m.name << "\"";

        if (i < len - 1) {
            stream << ",\n";
        }
    }

    stream << "\n\t]\n";

    stream << "}";
*/
    return true;
}

}  // namespace model
