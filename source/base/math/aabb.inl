#ifndef SU_BASE_MATH_AABB_INL
#define SU_BASE_MATH_AABB_INL

#include "aabb.hpp"
#include "matrix4x4.inl"
#include "vector3.inl"

namespace math {

inline constexpr AABB::AABB(float3 const& min, float3 const& max) noexcept : bounds{min, max} {}


inline float3 const& AABB::min() const noexcept {
    return bounds[0];
}

inline float3 const& AABB::max() const noexcept {
    return bounds[1];
}

inline float3 AABB::position() const noexcept {
    return 0.5f * (bounds[0] + bounds[1]);
}

inline float3 AABB::halfsize() const noexcept {
    return 0.5f * (bounds[1] - bounds[0]);
}

inline float3 AABB::extent() const noexcept {
    return bounds[1] - bounds[0];
}

inline float AABB::surface_area() const noexcept {
    float3 const d = bounds[1] - bounds[0];
    return 2.f * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
}

inline float AABB::volume() const noexcept {
    float3 const d = bounds[1] - bounds[0];
    return d[0] * d[1] * d[2];
}

inline bool AABB::intersect(float3 const& p) const noexcept {
    if (p[0] >= bounds[0][0] && p[0] <= bounds[1][0] && p[1] >= bounds[0][1] &&
        p[1] <= bounds[1][1] && p[2] >= bounds[0][2] && p[2] <= bounds[1][2]) {
        return true;
    }

    return false;
}

inline void AABB::set_min_max(float3 const& min, float3 const& max) noexcept {
    bounds[0] = min;
    bounds[1] = max;
}


inline void AABB::insert(float3 const& p) noexcept {
    bounds[0] = math::min(p, bounds[0]);
    bounds[1] = math::max(p, bounds[1]);
}

inline void AABB::scale(float x) noexcept {
    float3 const v = x * halfsize();
    bounds[0] -= v;
    bounds[1] += v;
}

inline void AABB::add(float x) noexcept {
    float3 const v(x);
    bounds[0] -= v;
    bounds[1] += v;
}

inline AABB AABB::transform(float4x4 const& m) const noexcept {
    float3 const mx = m.x();
    float3 const xa = bounds[0][0] * mx;
    float3 const xb = bounds[1][0] * mx;

    float3 const my = m.y();
    float3 const ya = bounds[0][1] * my;
    float3 const yb = bounds[1][1] * my;

    float3 const mz = m.z();
    float3 const za = bounds[0][2] * mz;
    float3 const zb = bounds[1][2] * mz;

    float3 const mw = m.w();

    return AABB((math::min(xa, xb) + math::min(ya, yb)) + (math::min(za, zb) + mw),
                (math::max(xa, xb) + math::max(ya, yb)) + (math::max(za, zb) + mw));
}

inline AABB AABB::transform_transposed(float3x3 const& m) const noexcept {
    /*
        float3 const mx(m.r[0][0], m.r[1][0], m.r[2][0]);
        float3 const xa = bounds[0][0] * mx;
        float3 const xb = bounds[1][0] * mx;

        float3 const my(m.r[0][1], m.r[1][1], m.r[2][1]);
        float3 const ya = bounds[0][1] * my;
        float3 const yb = bounds[1][1] * my;

        float3 const mz(m.r[0][2], m.r[1][2], m.r[2][2]);
        float3 const za = bounds[0][2] * mz;
        float3 const zb = bounds[1][2] * mz;

        return AABB(math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb),
                    math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb));
    */

    float3 const mx(m.r[0][0], m.r[1][0], m.r[2][0]);
    float3 const xa = bounds[0][0] * mx;
    float3 const xb = bounds[1][0] * mx;

    float3 const my(m.r[0][1], m.r[1][1], m.r[2][1]);
    float3 const ya = bounds[0][1] * my;
    float3 const yb = bounds[1][1] * my;

    float3 const mz(m.r[0][2], m.r[1][2], m.r[2][2]);
    float3 const za = bounds[0][2] * mz;
    float3 const zb = bounds[1][2] * mz;

    float3 const min = math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb);
    float3 const max = math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb);

    float3 const halfsize = 0.5f * (max - min);

    float3 const p = position();

    return AABB(p - halfsize, p + halfsize);
}

inline AABB AABB::merge(AABB const& other) const noexcept {
    return AABB(math::min(bounds[0], other.bounds[0]), math::max(bounds[1], other.bounds[1]));
}

inline void AABB::merge_assign(AABB const& other) noexcept {
    bounds[0] = math::min(bounds[0], other.bounds[0]);
    bounds[1] = math::max(bounds[1], other.bounds[1]);
}

inline void AABB::clip_min(float d, uint8_t axis) noexcept {
    bounds[0].v[axis] = std::max(d, bounds[0][axis]);
}

inline void AABB::clip_max(float d, uint8_t axis) noexcept {
    bounds[1].v[axis] = std::min(d, bounds[1][axis]);
}

inline bool AABB::operator==(AABB const& other) const noexcept {
    return bounds[0] == other.bounds[0] && bounds[1] == other.bounds[1];
}

inline constexpr AABB AABB::empty() noexcept {
    float constexpr max = std::numeric_limits<float>::max();
    return AABB(float3(max), float3(-max));
}

inline constexpr AABB AABB::infinite() noexcept {
    float constexpr max = std::numeric_limits<float>::max();
    return AABB(float3(-max), float3(max));
}

}  // namespace math

#endif
