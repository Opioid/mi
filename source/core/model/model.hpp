#ifndef SU_CORE_MODEL_MODEL_HPP
#define SU_CORE_MODEL_MODEL_HPP

#include "base/flags/flags.hpp"
#include "base/math/aabb.hpp"
#include "base/math/quaternion.hpp"
#include "base/math/vector3.hpp"

#include <cstdint>
#include <string>

struct aiMaterial;

namespace model {
class Model {
  public:
    enum class Transformation {
        Swap_XY   = 1 << 0,
        Swap_YZ   = 1 << 1,
        Reverse_X = 1 << 2,
        Reverse_Y = 1 << 3,
        Reverse_Z = 1 << 4

    };

    enum class Origin { Default = 0, Center_bottom };

    struct Part {
        uint32_t start_index;
        uint32_t num_indices;
        uint32_t material_index;
    };

    struct Material {
        bool empty() const noexcept {
            return roughness < 0.f;
        }

        std::string name;

        std::string mask_texture;
        std::string color_texture;
        std::string normal_texture;
        std::string roughness_texture;
        std::string specular_texture;
        std::string shininess_texture;
        std::string emission_texture;

        float3 diffuse_color = float3(0.75f);

        float3 emissive_color = float3(0.f);

        float roughness = -1.f;

        bool two_sided = false;
    };

    ~Model() noexcept;

    uint32_t num_parts() const noexcept;

    uint32_t num_materials() const noexcept;

    uint32_t num_vertices() const noexcept;

    uint32_t num_indices() const noexcept;

    Part const* parts() const noexcept;

    Material const* materials() const noexcept;

    float3 const* positions() const noexcept;

    float3 const* normals() const noexcept;

    float4 const* tangents() const noexcept;

    float2 const* texture_coordinates() const noexcept;

    uint32_t const* indices() const noexcept;

    void allocate_parts(uint32_t num_parts) noexcept;

    void allocate_materials(uint32_t num_materials) noexcept;

    void set_num_vertices(uint32_t num_vertices) noexcept;

    void allocate_positions() noexcept;

    void allocate_normals() noexcept;

    void allocate_tangents() noexcept;

    void allocate_texture_coordinates() noexcept;

    void allocate_indices(uint32_t num_indices) noexcept;

    void set_part(uint32_t id, Part const& part) noexcept;

    void set_material(uint32_t id, aiMaterial const& material) noexcept;

    void set_position(uint32_t id, float3 const& p) noexcept;

    void set_normal(uint32_t id, float3 const& n) noexcept;

    void set_tangent(uint32_t id, float3 const& t, float3 const& b, float3 const& n) noexcept;

    void set_tangent(uint32_t id, float3 const& t, float3 const& n, float bitangent_sign) noexcept;

    void set_texture_coordinate(uint32_t id, float2 uv) noexcept;

    void set_index(uint32_t id, uint32_t index) noexcept;

    void scale(float3 const& s) noexcept;

    void transform(flags::Flags<Transformation> transformtions) noexcept;

    void set_origin(Origin origin) noexcept;

    AABB aabb() const noexcept;

    void try_to_fix_tangent_space();

    static Quaternion tangent_space(float3 const& t, float3 const& n, float bitangent_sign);

  private:
    uint32_t num_parts_ = 0;

    uint32_t num_materials_ = 0;

    Part* parts_ = nullptr;

    Material* materials_ = nullptr;

    uint32_t num_vertices_ = 0;

    uint32_t num_indices_ = 0;

    float3* positions_ = nullptr;

    float3* normals_ = nullptr;

    float4* tangents_and_bitangent_signs_ = nullptr;

    float2* texture_coordinates_ = nullptr;

    uint32_t* indices_;
};
}  // namespace model

#endif
