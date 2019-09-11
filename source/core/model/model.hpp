#ifndef SU_CORE_MODEL_MODEL_HPP
#define SU_CORE_MODEL_MODEL_HPP

#include "base/math/vector.hpp"

#include <cstdint>

namespace model {
class Model {
public:

    struct Part {
        uint32_t start_index;
        uint32_t num_indices;
        uint32_t material_index;
    };

    ~Model() noexcept;

    uint32_t num_parts() const noexcept;

    uint32_t num_vertices() const noexcept;

    uint32_t num_indices() const noexcept;

    Part const* parts() const noexcept;

    float3 const* positions() const noexcept;

    float2 const* texture_coordinates() const noexcept;

    float3 const* normals() const noexcept;

    float4 const* tangents() const noexcept;

    uint32_t const* indices() const noexcept;

    void allocate_parts(uint32_t num_parts) noexcept;

    void set_num_vertices(uint32_t num_vertices) noexcept;

    void allocate_positions() noexcept;

    void allocate_texture_coordinates() noexcept;

    void allocate_normals() noexcept;

    void allocate_tangents() noexcept;

    void allocate_indices(uint32_t num_indices) noexcept;

    void set_part(uint32_t id, Part const& part) noexcept;

        void set_position(uint32_t id, float3 const& p) noexcept;

 void set_texture_coordinate(uint32_t id, float2 uv) noexcept;

     void set_normal(uint32_t id, float3 const& n) noexcept;

    void set_tangent(uint32_t id, float3 const& t, float3 const& b, float3 const& n) noexcept;

    void set_index(uint32_t id, uint32_t index) noexcept;

    uint32_t num_parts_ = 0;

    Part* parts_ = nullptr;

    uint32_t num_vertices_ = 0;

    uint32_t num_indices_ = 0;

    float3* positions_ = nullptr;

    float2* texture_coordinates_ = nullptr;

    float3* normals_ = nullptr;

    float4* tangents_and_bitangent_signs_ = nullptr;

    uint32_t* indices_;
};
}

#endif
