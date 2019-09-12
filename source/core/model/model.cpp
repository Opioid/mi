#include "model.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector4.inl"

#include <assimp/scene.h>

namespace model {

Model::~Model() noexcept {
    delete[] indices_;

    delete[] tangents_and_bitangent_signs_;
    delete[] normals_;
    delete[] texture_coordinates_;
    delete[] positions_;

    delete[] materials_;
    delete[] parts_;
}

uint32_t Model::num_parts() const noexcept {
    return num_parts_;
}

uint32_t Model::num_materials() const noexcept {
    return num_materials_;
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

Model::Material const* Model::materials() const noexcept {
    return materials_;
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
    parts_     = new Part[num_parts];
}

void Model::allocate_materials(uint32_t num_materials) noexcept {
    num_materials_ = num_materials;
    materials_     = new Material[num_materials];
}

void Model::set_num_vertices(uint32_t num_vertices) noexcept {
    num_vertices_ = num_vertices;
}

void Model::allocate_positions() noexcept {
    positions_ = new float3[num_vertices_];
}

void Model::allocate_texture_coordinates() noexcept {
    texture_coordinates_ = new float2[num_vertices_];
}
void Model::allocate_normals() noexcept {
    normals_ = new float3[num_vertices_];
}
void Model::allocate_tangents() noexcept {
    tangents_and_bitangent_signs_ = new float4[num_vertices_];
}

void Model::allocate_indices(uint32_t num_indices) noexcept {
    num_indices_ = num_indices;

    indices_ = new uint32_t[num_indices];
}

void Model::set_part(uint32_t id, Part const& part) noexcept {
    parts_[id] = part;
}

static inline float shininess_to_roughness(float shininess) noexcept {
    return std::pow(2.f / (shininess + 2.f), 0.25f);
}

static inline float3 aiColor3D_to_float3(aiColor3D const& c) noexcept {
    return float3(c.r, c.g, c.b);
}

static inline std::string aiTextureType_to_string(aiMaterial const& material,
                                                  aiTextureType     type) noexcept {
    std::string name;

    if (aiString path; aiReturn_SUCCESS == material.GetTexture(type, 0, &path)) {
        name = path.data;
        std::replace(name.begin(), name.end(), '\\', '/');
    }

    return name;
}

void Model::set_material(uint32_t id, aiMaterial const& material) noexcept {
    Material& m = materials_[id];

    if (aiString text; aiReturn_SUCCESS == material.Get(AI_MATKEY_NAME, text)) {
        m.name = text.data;
    }

    m.mask_texture = aiTextureType_to_string(material, aiTextureType_OPACITY);

    m.color_texture = aiTextureType_to_string(material, aiTextureType_BASE_COLOR);

    if (m.color_texture.empty()) {
        m.color_texture = aiTextureType_to_string(material, aiTextureType_DIFFUSE);
    }

    m.normal_texture = aiTextureType_to_string(material, aiTextureType_NORMALS);

    m.roughness_texture = aiTextureType_to_string(material, aiTextureType_DIFFUSE_ROUGHNESS);
    m.shininess_texture = aiTextureType_to_string(material, aiTextureType_SHININESS);

    if (aiColor3D diffuse_color;
        aiReturn_SUCCESS == material.Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color)) {
        m.diffuse_color = aiColor3D_to_float3(diffuse_color);
    }

    float shininess = -1.f;
    material.Get(AI_MATKEY_SHININESS, shininess);

    float roughness = 0.75f;
    if (shininess > 0.f) {
        roughness = shininess_to_roughness(shininess);
    }

    m.roughness = roughness;
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

void Model::transform(flags::Flags<Transformation> transformtions) noexcept {}

AABB Model::aabb() const noexcept {
    AABB box = AABB::empty();

    for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
        float3 const p = positions_[i];

        box.bounds[0] = min(box.bounds[0], p);
        box.bounds[1] = max(box.bounds[1], p);
    }

    return box;
}

}  // namespace model
