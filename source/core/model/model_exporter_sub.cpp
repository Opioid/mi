#include "model_exporter_sub.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "model.hpp"
#include "rapidjson/prettywriter.h"

#include <fstream>

namespace model {

struct Vertex {
    packed_float3 p;
    packed_float3 n;
    packed_float3 t;

    float2 uv;

    uint8_t bitangent_sign;

    uint8_t pad[3];
};

struct Vertex_layout_description {
    enum class Encoding { UInt8, UInt16, UInt32, Float32, Float32x2, Float32x3, Float32x4 };

    struct Element {
        std::string semantic_name;
        uint32_t    semantic_index = 0;
        Encoding    encoding;
        uint32_t    stream      = 0;
        uint32_t    byte_offset = 0;
    };
};

static std::string to_string(Vertex_layout_description::Encoding const& encoding) {
    using Encoding = Vertex_layout_description::Encoding;

    switch (encoding) {
        case Encoding::UInt8:
            return "UInt8";
        case Encoding::Float32:
            return "Float32";
        case Encoding::Float32x2:
            return "Float32x2";
        case Encoding::Float32x3:
            return "Float32x3";
        case Encoding::Float32x4:
            return "Float32x4";
        default:
            return "Undefined";
    }
}

template <class Writer>
static void write(Writer& writer, Vertex_layout_description::Element const& element) {
    writer.StartObject();

    writer.Key("semantic_name");
    writer.String(element.semantic_name.c_str());

    writer.Key("semantic_index");
    writer.Uint(element.semantic_index);

    writer.Key("encoding");
    writer.String(to_string(element.encoding).c_str());

    writer.Key("stream");
    writer.Uint(element.stream);

    writer.Key("byte_offset");
    writer.Uint(element.byte_offset);

    writer.EndObject();
}

template <class Writer>
static void binary_tag(Writer& writer, uint64_t offset, uint64_t size) noexcept {
    writer.Key("binary");
    writer.StartObject();

    //    writer.Key("index");
    //    writer.Uint(0);

    writer.Key("offset");
    writer.Uint64(offset);

    writer.Key("size");
    writer.Uint64(size);

    writer.EndObject();
}

bool Exporter_sub::write(std::string const& name, Model const& model) const noexcept {
    std::ofstream stream(name + ".sub", std::ios::binary);

    if (!stream) {
        return false;
    }

    rapidjson::StringBuffer sb;

    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.StartObject();

    writer.Key("geometry");
    writer.StartObject();

    // Parts
    writer.Key("parts");
    writer.StartArray();

    Model::Part const* parts = model.parts();
    for (uint32_t i = 0, len = model.num_parts(); i < len; ++i) {
        writer.StartObject();

        writer.Key("start_index");
        writer.Uint(parts[i].start_index);

        writer.Key("num_indices");
        writer.Uint(parts[i].num_indices);

        writer.Key("material_index");
        writer.Uint(parts[i].material_index);

        writer.EndObject();
    }

    writer.EndArray();

    // Vertices
    writer.Key("vertices");
    writer.StartObject();

    static bool constexpr tangent_space_as_quaternion = true;
    static bool constexpr interleaved_vertex_stream   = false;

    bool const has_uvs_and_tangents = nullptr != model.texture_coordinates() &&
                                      nullptr != model.tangents();

    uint64_t vertex_size = 0;

    if (interleaved_vertex_stream) {
        vertex_size = sizeof(Vertex);
    } else {
        if (tangent_space_as_quaternion && has_uvs_and_tangents) {
            vertex_size = (3 * 4 + 4 * 4 + 4 * 2);
        } else {
            vertex_size = has_uvs_and_tangents ? (3 * 4 + 3 * 4 + 3 * 4 + 2 * 4 + 1)
                                               : (3 * 4 + 3 * 4);
        }
    }

    uint64_t const num_vertices  = model.num_vertices();
    uint64_t const vertices_size = num_vertices * vertex_size;

    binary_tag(writer, 0, vertices_size);

    writer.Key("num_vertices");
    writer.Uint64(num_vertices);

    writer.Key("layout");
    writer.StartArray();

    Vertex_layout_description::Element element;

    using Encoding = Vertex_layout_description::Encoding;

    element.semantic_name = "Position";
    element.encoding      = Encoding::Float32x3;
    element.stream        = 0;
    model::write(writer, element);

    if (tangent_space_as_quaternion && has_uvs_and_tangents) {
        element.semantic_name = "Tangent_space";
        element.encoding      = Encoding::Float32x4;
        element.stream        = 1;
        model::write(writer, element);

        element.semantic_name = "Texture_coordinate";
        element.encoding      = Encoding::Float32x2;
        element.stream        = 2;
        model::write(writer, element);

    } else {
        element.semantic_name = "Normal";
        element.stream        = 1;
        model::write(writer, element);

        if (has_uvs_and_tangents) {
            element.semantic_name = "Tangent";
            element.stream        = 2;
            model::write(writer, element);

            element.semantic_name = "Texture_coordinate";
            element.encoding      = Encoding::Float32x2;
            element.stream        = 3;
            model::write(writer, element);

            element.semantic_name = "Bitangent_sign";
            element.encoding      = Encoding::UInt8;
            element.stream        = 4;
            model::write(writer, element);
        }
    }

    writer.EndArray();

    // close vertices
    writer.EndObject();

    // Indices
    writer.Key("indices");
    writer.StartObject();

    int64_t max_index = 0;
    int64_t max_index_delta = 0;
    int64_t min_index_delta = 0;

    {
        int64_t previous_index = 0;

        uint32_t const* indices = model.indices();
        for (uint32_t i = 0, len = model.num_indices(); i < len; ++i) {
            int64_t const si = int64_t(indices[i]);

            max_index = std::max(max_index, si);

            int64_t const delta_index = si - previous_index;

            max_index_delta = std::max(delta_index, max_index_delta);
            min_index_delta = std::min(delta_index, min_index_delta);

            previous_index = si;
        }
    }

    bool   delta_indices = false;
    uint32_t index_bytes   = 4;

//    if (max_index <= 0x000000000000FFFF) {
//        index_bytes = 2;
//    }

//    if (max_index_delta <= 0x0000000000007FFF && std::abs(min_index_delta) <= 0x0000000000007FFF) {
//        index_bytes = 2;
//        delta_indices = true;
//    } else if (max_index_delta <= 0x000000007FFFFFFF && std::abs(min_index_delta) <= 0x000000007FFFFFFF) {
//        delta_indices = true;
//    }

    uint64_t const num_indices = model.num_indices();

    binary_tag(writer, vertices_size, num_indices * index_bytes);

    writer.Key("num_indices");
    writer.Uint64(num_indices);

    writer.Key("encoding");

    if (4 == index_bytes) {
        if (delta_indices) {
            writer.String("Int32");
        } else {
            writer.String("UInt32");
        }
    } else {
        if (delta_indices) {
            writer.String("Int16");
        } else {
            writer.String("UInt16");
        }
    }

    // close indices
    writer.EndObject();

    // close geometry
    writer.EndObject();

    // close start
    writer.EndObject();

    uint64_t const json_size         = sb.GetSize();
    uint64_t const aligned_json_size = json_size + json_size % 4;

    const char header[] = "SUB\000";
    stream.write(header, sizeof(char) * 4);

    stream.write(reinterpret_cast<char const*>(&aligned_json_size), sizeof(uint64_t));
    stream.write(reinterpret_cast<char const*>(sb.GetString()), json_size * sizeof(char));

    for (int64_t i = aligned_json_size - json_size; i > 0; --i) {
        stream.put(0);
    }

    // binary stuff

    if (interleaved_vertex_stream) {
        memory::Buffer<Vertex> vertices(num_vertices);

        for (uint32_t i = 0, len = model.num_vertices(); i < len; ++i) {
            Vertex& v = vertices[i];

            v.p = packed_float3(model.positions()[i]);

            if (model.normals()) {
                v.n = packed_float3(model.normals()[i]);
            } else {
                v.n = packed_float3(0.f);
            }

            if (model.tangents()) {
                v.t              = packed_float3(model.tangents()[i].xyz());
                v.bitangent_sign = model.tangents()[i][3] < 0.f ? 1 : 0;
            } else {
                v.t              = packed_float3(0.f);
                v.bitangent_sign = 0;
            }

            if (model.texture_coordinates()) {
                v.uv = model.texture_coordinates()[i];
            } else {
                v.uv = float2(0.f);
            }

            v.pad[0] = 0;
            v.pad[1] = 0;
            v.pad[2] = 0;
        }

        stream.write(reinterpret_cast<char const*>(vertices.data()), vertices_size);
    } else {
        memory::Buffer<uint8_t> buffer(num_vertices * 4 * sizeof(float));

        packed_float3* floats3 = reinterpret_cast<packed_float3*>(buffer.data());

        float3 const* positions = model.positions();
        for (uint32_t i = 0; i < num_vertices; ++i) {
            floats3[i] = packed_float3(positions[i]);
        }

        stream.write(reinterpret_cast<char const*>(floats3), num_vertices * sizeof(packed_float3));

        if (tangent_space_as_quaternion && has_uvs_and_tangents) {
            float4* floats4 = reinterpret_cast<float4*>(buffer.data());

            float4 const* tangents = model.tangents();
            float3 const* normals  = model.normals();

            for (uint32_t i = 0; i < num_vertices; ++i) {
                float4 const t = tangents[i];
                float3 const n = normals[i];

                Quaternion const ts = Model::tangent_space(t.xyz(), n, t[3]);

                floats4[i] = ts;
            }

            stream.write(reinterpret_cast<char const*>(floats4), num_vertices * sizeof(float4));

            float2 const* uvs = model.texture_coordinates();

            stream.write(reinterpret_cast<char const*>(uvs), num_vertices * sizeof(float2));
        } else {
            float3 const* normals = model.normals();
            for (uint32_t i = 0; i < num_vertices; ++i) {
                if (normals) {
                    floats3[i] = packed_float3(normals[i]);
                } else {
                    floats3[i] = packed_float3(0.f);
                }
            }

            stream.write(reinterpret_cast<char const*>(floats3),
                         num_vertices * sizeof(packed_float3));

            if (has_uvs_and_tangents) {
                float4 const* tangents = model.tangents();
                for (uint32_t i = 0; i < num_vertices; ++i) {
                    if (tangents) {
                        floats3[i] = packed_float3(tangents[i].xyz());
                    } else {
                        floats3[i] = packed_float3(0.f);
                    }
                }

                stream.write(reinterpret_cast<char const*>(floats3),
                             num_vertices * sizeof(packed_float3));

                float2* floats2 = reinterpret_cast<float2*>(buffer.data());

                float2 const* uvs = model.texture_coordinates();
                for (uint32_t i = 0; i < num_vertices; ++i) {
                    if (uvs) {
                        floats2[i] = uvs[i];
                    } else {
                        floats2[i] = float2(0.f);
                    }
                }

                stream.write(reinterpret_cast<char const*>(floats2), num_vertices * sizeof(float2));

                uint8_t* bytes = reinterpret_cast<uint8_t*>(buffer.data());

                for (uint32_t i = 0; i < num_vertices; ++i) {
                    if (tangents) {
                        bytes[i] = tangents[i][3] < 0.f ? 1 : 0;
                    } else {
                        bytes[i] = 0;
                    }
                }

                stream.write(reinterpret_cast<char const*>(bytes), num_vertices * sizeof(uint8_t));
            }
        }
    }

    uint32_t const* indices = model.indices();

    if (4 == index_bytes) {
        int32_t previous_index = 0;

        if (delta_indices) {
            for (uint32_t i = 0; i < num_indices; ++i) {
                int32_t const a = int32_t(indices[i]);

                int32_t const delta_index = a - previous_index;
                stream.write(reinterpret_cast<char const*>(&delta_index), sizeof(int32_t));

                previous_index = a;
            }
        } else {
            stream.write(reinterpret_cast<char const*>(model.indices()),
                         num_indices * sizeof(uint32_t));
        }
    } else {
        if (delta_indices) {
            int32_t previous_index = 0;

            for (uint32_t i = 0; i < num_indices; ++i) {
                int32_t const a = int32_t(indices[i]);

                int16_t const delta_index = int16_t(a - previous_index);
                stream.write(reinterpret_cast<char const*>(&delta_index), sizeof(int16_t));

                previous_index = a;
            }
        } else {
            for (uint32_t i = 0; i < num_indices; ++i) {
                uint16_t const a = uint16_t(indices[i]);

                stream.write(reinterpret_cast<char const*>(&a), sizeof(uint16_t));
            }
        }
    }

    return true;
}

}  // namespace model
