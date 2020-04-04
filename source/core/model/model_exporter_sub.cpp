#include "model_exporter_sub.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "model.hpp"

#include <fstream>
#include <sstream>

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

std::stringstream& operator<<(std::stringstream&                        stream,
                              Vertex_layout_description::Element const& element);

static void newline(std::ostream& stream, uint32_t num_tabs) noexcept {
    stream << std::endl;

    for (uint32_t i = 0; i < num_tabs; ++i) {
        stream << '\t';
    }
}

static void binary_tag(std::ostream& stream, uint64_t offset, uint64_t size) noexcept {
    stream << "\"binary\":{\"offset\":" << offset << ",\"size\":" << size << "}";
}

bool Exporter_sub::write(std::string const& name, Model const& model) const noexcept {
    std::ofstream stream(name + ".sub", std::ios::binary);

    if (!stream) {
        return false;
    }

    std::stringstream jstream;

    newline(jstream, 0);
    jstream << "{";

    newline(jstream, 1);
    jstream << "\"geometry\":{";

    newline(jstream, 2);
    jstream << "\"parts\":[";

    Model::Part const* parts = model.parts();
    for (uint32_t i = 0, len = model.num_parts(); i < len; ++i) {
        newline(jstream, 3);

        auto const& p = parts[i];
        jstream << "{";
        jstream << "\"start_index\":" << p.start_index << ",";
        jstream << "\"num_indices\":" << p.num_indices << ",";
        jstream << "\"material_index\":" << p.material_index;
        jstream << "}";

        if (i < len - 1) {
            jstream << ",";
        }
    }

    // close parts
    newline(jstream, 2);
    jstream << "],";

    // vertices
    newline(jstream, 2);
    jstream << "\"vertices\":{";

    newline(jstream, 3);

    static bool constexpr tangent_space_as_quaternion = true;
    static bool constexpr interleaved_vertex_stream   = false;

    bool const has_uvs_and_tangents = nullptr != model.texture_coordinates() &&
                                      nullptr != model.tangents();

    uint64_t vertex_size = 0;

    if (interleaved_vertex_stream) {
        vertex_size = sizeof(Vertex);
    } else {
        if (tangent_space_as_quaternion) {
            vertex_size = (3 * 4 + 4 * 4 + 4 * 2);
        } else {
            vertex_size = has_uvs_and_tangents ? (3 * 4 + 3 * 4 + 3 * 4 + 2 * 4 + 1)
                                               : (3 * 4 + 3 * 4);
        }
    }

    uint64_t const num_vertices  = model.num_vertices();
    uint64_t const vertices_size = num_vertices * vertex_size;

    binary_tag(jstream, 0, vertices_size);
    jstream << ",";

    newline(jstream, 3);
    jstream << "\"num_vertices\":" << num_vertices << ",";

    newline(jstream, 3);
    jstream << "\"layout\":[";

    Vertex_layout_description::Element element;

    using Encoding = Vertex_layout_description::Encoding;

    if (interleaved_vertex_stream) {
        newline(jstream, 4);
        element.semantic_name = "Position";
        element.encoding      = Encoding::Float32x3;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Normal";
        element.byte_offset   = 12;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Tangent";
        element.byte_offset   = 24;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Texture_coordinate";
        element.encoding      = Encoding::Float32x2;
        element.byte_offset   = 36;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Bitangent_sign";
        element.encoding      = Encoding::UInt8;
        element.byte_offset   = 44;
        jstream << element;
    } else {
        newline(jstream, 4);
        element.semantic_name = "Position";
        element.encoding      = Encoding::Float32x3;
        element.stream        = 0;
        jstream << element << ",";

        if (tangent_space_as_quaternion && has_uvs_and_tangents) {
            newline(jstream, 4);
            element.semantic_name = "Tangent_space";
            element.encoding      = Encoding::Float32x4;
            element.stream        = 1;
            jstream << element << ",";

            newline(jstream, 4);
            element.semantic_name = "Texture_coordinate";
            element.encoding      = Encoding::Float32x2;
            element.stream        = 2;
            jstream << element;

        } else {
            newline(jstream, 4);
            element.semantic_name = "Normal";
            element.stream        = 1;
            jstream << element;

            if (has_uvs_and_tangents) {
                jstream << ",";

                newline(jstream, 4);
                element.semantic_name = "Tangent";
                element.stream        = 2;
                jstream << element << ",";

                newline(jstream, 4);
                element.semantic_name = "Texture_coordinate";
                element.encoding      = Encoding::Float32x2;
                element.stream        = 3;
                jstream << element << ",";

                newline(jstream, 4);
                element.semantic_name = "Bitangent_sign";
                element.encoding      = Encoding::UInt8;
                element.stream        = 4;
                jstream << element;
            }
        }
    }

    // close layout
    newline(jstream, 3);
    jstream << "]";

    // close vertices
    newline(jstream, 2);
    jstream << "},";

    // indices
    newline(jstream, 2);
    jstream << "\"indices\":{";

    int64_t max_index_delta = 0;
    int64_t min_index_delta = 0;

    {
        int64_t previous_index = 0;

        uint32_t const* indices = model.indices();
        for (uint32_t i = 0, len = model.num_indices(); i < len; ++i) {
            int64_t const si = int64_t(indices[i]);

            int64_t const delta_index = si - previous_index;

            max_index_delta = std::max(delta_index, max_index_delta);
            min_index_delta = std::min(delta_index, min_index_delta);

            previous_index = si;
        }
    }

    bool   delta_indices = false;
    size_t index_bytes   = 4;

    if (max_index_delta <= 0x000000000000FFFF && std::abs(min_index_delta) <= 0x000000000000FFFF) {
        if (max_index_delta <= 0x0000000000007FFF) {
            delta_indices = true;
        }

        index_bytes = 2;
    } else if (max_index_delta <= 0x000000007FFFFFFF) {
        delta_indices = true;
    }

    newline(jstream, 3);
    uint64_t const num_indices = model.num_indices();
    binary_tag(jstream, vertices_size, num_indices * index_bytes);
    jstream << ",";

    newline(jstream, 3);
    jstream << "\"num_indices\":" << num_indices << ",";

    newline(jstream, 3);
    jstream << "\"encoding\":";

    if (4 == index_bytes) {
        if (delta_indices) {
            jstream << "\"Int32\"";
        } else {
            jstream << "\"UInt32\"";
        }
    } else {
        if (delta_indices) {
            jstream << "\"Int16\"";
        } else {
            jstream << "\"UInt16\"";
        }
    }

    // close indices
    newline(jstream, 2);
    jstream << "}";

    // close geometry
    newline(jstream, 1);
    jstream << "}";

    // close start
    newline(jstream, 0);
    jstream << "}";

    newline(jstream, 0);

    std::string const json_string = jstream.str();
    uint64_t const    json_size   = json_string.size() - 1;

    const char header[] = "SUB\000";
    stream.write(header, sizeof(char) * 4);

    stream.write(reinterpret_cast<char const*>(&json_size), sizeof(uint64_t));
    stream.write(reinterpret_cast<char const*>(json_string.data()), json_size * sizeof(char));

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
                int32_t const a = static_cast<int32_t>(indices[i]);

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

using Encoding = Vertex_layout_description::Encoding;

void print(std::stringstream& stream, Encoding encoding) {
    switch (encoding) {
        case Encoding::UInt8:
            stream << "UInt8";
            break;
        case Encoding::Float32:
            stream << "Float32";
            break;
        case Encoding::Float32x2:
            stream << "Float32x2";
            break;
        case Encoding::Float32x3:
            stream << "Float32x3";
            break;
        case Encoding::Float32x4:
            stream << "Float32x4";
            break;
        default:
            stream << "Undefined";
    }
}

std::stringstream& operator<<(std::stringstream&                        stream,
                              Vertex_layout_description::Element const& element) {
    stream << "{";
    stream << "\"semantic_name\":\"" << element.semantic_name << "\",";
    stream << "\"semantic_index\":" << element.semantic_index << ",";

    stream << "\"encoding\":\"";
    print(stream, element.encoding);
    stream << "\",";
    stream << "\"stream\":" << element.stream << ",";
    stream << "\"byte_offset\":" << element.byte_offset;

    stream << "}";

    return stream;
}

}  // namespace model
