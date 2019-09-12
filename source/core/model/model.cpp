#include "model.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"

namespace model {

Model::~Model() noexcept {
    memory::free_aligned(indices_);

    memory::free_aligned(tangents_and_bitangent_signs_);
    memory::free_aligned(normals_);
    memory::free_aligned(texture_coordinates_);
    memory::free_aligned(positions_);

    memory::free_aligned(parts_);
}

uint32_t Model::num_parts() const noexcept {
    return num_parts_;
}

uint32_t Model::num_vertices() const noexcept {
    return num_vertices_;
}

uint32_t Model::num_indices() const noexcept {
    return num_indices_;
}

Model::Part const* Model::parts() const noexcept {
    return parts_;
}

float3 const* Model::positions() const noexcept {
    return positions_;
}

float2 const* Model::texture_coordinates() const noexcept {
    return texture_coordinates_;
}

float3 const* Model::normals() const noexcept {
    return normals_;
}

float4 const* Model::tangents() const noexcept {
    return tangents_and_bitangent_signs_;
}

uint32_t const* Model::indices() const noexcept {
    return indices_;
}

void Model::allocate_parts(uint32_t num_parts) noexcept {
    num_parts_ = num_parts;
    parts_     = memory::allocate_aligned<Part>(num_parts);
}

void Model::set_num_vertices(uint32_t num_vertices) noexcept {
    num_vertices_ = num_vertices;
}

void Model::allocate_positions() noexcept {
    positions_ = memory::allocate_aligned<float3>(num_vertices_);
}

void Model::allocate_texture_coordinates() noexcept {
    texture_coordinates_ = memory::allocate_aligned<float2>(num_vertices_);
}
void Model::allocate_normals() noexcept {
    normals_ = memory::allocate_aligned<float3>(num_vertices_);
}
void Model::allocate_tangents() noexcept {
    tangents_and_bitangent_signs_ = memory::allocate_aligned<float4>(num_vertices_);
}

void Model::allocate_indices(uint32_t num_indices) noexcept {
    num_indices_ = num_indices;

    indices_ = memory::allocate_aligned<uint32_t>(num_indices);
}

void Model::set_part(uint32_t id, Part const& part) noexcept {
    parts_[id] = part;
}

void Model::set_position(uint32_t id, float3 const& p) noexcept {
    positions_[id] = p;
}

void Model::set_texture_coordinate(uint32_t id, float2 uv) noexcept {
    texture_coordinates_[id] = uv;
}

void Model::set_normal(uint32_t id, float3 const& n) noexcept {
    normals_[id] = n;
}

void Model::set_tangent(uint32_t id, float3 const& t, float3 const& b, float3 const& n) noexcept {
    normals_[id] = n;

    float3 const b2 = cross(t, n);

    float const s = dot(b, b2);

    tangents_and_bitangent_signs_[id] = float4(t, s > 0.f ? 1.f : -1.f);
}

void Model::set_index(uint32_t id, uint32_t index) noexcept {
    indices_[id] = index;
}

void Model::scale(float3 const& s) noexcept {
    for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
        positions_[i] *= s;
    }
}

}  // namespace model
