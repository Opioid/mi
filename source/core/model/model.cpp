#include "model.hpp"
#include "base/math/aabb.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector4.inl"

#include <assimp/scene.h>

namespace model {

Model::~Model() noexcept {
    delete[] indices_;

    delete[] texture_coordinates_;
    delete[] tangents_and_bitangent_signs_;
    delete[] normals_;

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

float3 const* Model::normals() const noexcept {
    return normals_;
}

float4 const* Model::tangents() const noexcept {
    return tangents_and_bitangent_signs_;
}

float2 const* Model::texture_coordinates() const noexcept {
    return texture_coordinates_;
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

void Model::allocate_normals() noexcept {
    normals_ = new float3[num_vertices_];
}

void Model::allocate_tangents() noexcept {
    tangents_and_bitangent_signs_ = new float4[num_vertices_];
}

void Model::allocate_texture_coordinates() noexcept {
    texture_coordinates_ = new float2[num_vertices_];
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

    if (aiColor3D color; aiReturn_SUCCESS == material.Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
        m.diffuse_color = aiColor3D_to_float3(color);
    }

    if (aiColor3D color; aiReturn_SUCCESS == material.Get(AI_MATKEY_COLOR_EMISSIVE, color)) {
        m.emissive_color = aiColor3D_to_float3(color);
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

void Model::set_tangent(uint32_t id, float3 const& t, float3 const& n,
                        float bitangent_sign) noexcept {
    normals_[id] = n;

    tangents_and_bitangent_signs_[id] = float4(t, bitangent_sign);
}

void Model::set_index(uint32_t id, uint32_t index) noexcept {
    indices_[id] = index;
}

void Model::scale(float3 const& s) noexcept {
    for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
        positions_[i] *= s;
    }
}

void Model::transform(flags::Flags<Transformation> transformtions) noexcept {
    if (transformtions.empty()) {
        return;
    }

    if (positions_) {
        for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
            if (transformtions.is(Transformation::Reverse_X)) {
                positions_[i][0] = -positions_[i][0];
            }

            if (transformtions.is(Transformation::Reverse_Z)) {
                positions_[i][2] = -positions_[i][2];
            }
        }
    }

    if (normals_) {
        for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
            if (transformtions.is(Transformation::Reverse_X)) {
                normals_[i][0] = -normals_[i][0];
            }

            if (transformtions.is(Transformation::Reverse_Z)) {
                normals_[i][2] = -normals_[i][2];
            }
        }
    }

    if (tangents_and_bitangent_signs_) {
        for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
            if (transformtions.is(Transformation::Reverse_X)) {
                tangents_and_bitangent_signs_[i][0] = -tangents_and_bitangent_signs_[i][0];
            }

            if (transformtions.is(Transformation::Reverse_Z)) {
                tangents_and_bitangent_signs_[i][2] = -tangents_and_bitangent_signs_[i][2];
            }
        }
    }

    if ((transformtions.is(Transformation::Reverse_X) &&
         transformtions.no(Transformation::Reverse_Z)) ||
        (transformtions.is(Transformation::Reverse_Z) &&
         transformtions.no(Transformation::Reverse_X))) {
        for (uint32_t i = 0, len = num_indices_; i < len; i += 3) {
            std::swap(indices_[i + 1], indices_[i + 2]);
        }
    }
}

void Model::set_origin(Origin origin) noexcept {
    if (Origin::Default == origin) {
        return;
    }

    if (Origin::Center_bottom == origin) {
        AABB const box = aabb();

        float3 const position = box.position();
        float3 const halfsize = box.halfsize();

        float3 const offset = float3(-position[0], halfsize[1] - position[1], -position[2]);

        for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
            positions_[i] += offset;
        }
    }
}

AABB Model::aabb() const noexcept {
    AABB box = AABB::empty();

    for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
        float3 const p = positions_[i];

        box.bounds[0] = min(box.bounds[0], p);
        box.bounds[1] = max(box.bounds[1], p);
    }

    return box;
}

Quaternion Model::tangent_space(float3 const& t, float3 const& n, float bitangent_sign) {
    float3 const b = cross(n, t);

    float3x3 const tbn(t, b, n);

    Quaternion q = quaternion::create(tbn);

    static float constexpr threshold   = 0.000001f;
    static float const renormalization = std::sqrt(1.f - threshold * threshold);

    if (std::abs(q[3]) < threshold) {
        q[0] *= renormalization;
        q[1] *= renormalization;
        q[2] *= renormalization;
        q[3] = q[3] < 0.f ? -threshold : threshold;
    }

    if (q[3] < 0.f) {
        q = -q;
    }

    if (bitangent_sign < 0.f) {
        q[3] = -q[3];
    }

    return q;
}

}  // namespace model
