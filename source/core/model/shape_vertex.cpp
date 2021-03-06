#include "shape_vertex.hpp"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"

namespace model {

size_t Vertex::unpadded_size() {
    return 3 * 4 + 3 * 4 + 3 * 4 + 2 * 4 + 1;
}

Vertex_stream::~Vertex_stream() = default;

Vertex_stream::Vertex_stream(uint32_t num_vertices) : num_vertices_(num_vertices) {}

uint32_t Vertex_stream::num_vertices() const {
    return num_vertices_;
}

Vertex_stream_interleaved::Vertex_stream_interleaved(uint32_t num_vertices, Vertex const* vertices)
    : Vertex_stream(num_vertices), vertices_(vertices) {}

void Vertex_stream_interleaved::release() {
    delete[] vertices_;
}

float3 Vertex_stream_interleaved::p(uint32_t i) const {
    return float3(vertices_[i].p);
}

Vertex_stream::NT Vertex_stream_interleaved::nt(uint32_t i) const {
    return {float3(vertices_[i].n), float3(vertices_[i].t)};
}

float2 Vertex_stream_interleaved::uv(uint32_t i) const {
    return vertices_[i].uv;
}

bool Vertex_stream_interleaved::bitangent_sign(uint32_t i) const {
    return vertices_[i].bitangent_sign > 0;
}

Vertex_stream_separate::Vertex_stream_separate(uint32_t num_vertices, packed_float3 const* p,
                                               packed_float3 const* n, packed_float3 const* t,
                                               float2 const* uv, uint8_t const* bts)
    : Vertex_stream(num_vertices), p_(p), n_(n), t_(t), uv_(uv), bts_(bts) {}

void Vertex_stream_separate::release() {
    delete[] p_;
    delete[] n_;
    delete[] t_;
    delete[] uv_;
    delete[] bts_;
}

float3 Vertex_stream_separate::p(uint32_t i) const {
    return float3(p_[i]);
}

Vertex_stream::NT Vertex_stream_separate::nt(uint32_t i) const {
    return {float3(n_[i]), float3(t_[i])};
}

float2 Vertex_stream_separate::uv(uint32_t i) const {
    return uv_[i];
}

bool Vertex_stream_separate::bitangent_sign(uint32_t i) const {
    return bts_[i] > 0;
}

Vertex_stream_separate_ts::Vertex_stream_separate_ts(uint32_t num_vertices, packed_float3 const* p,
                                                     Quaternion const* ts, float2 const* uv)
    : Vertex_stream(num_vertices), p_(p), ts_(ts), uv_(uv) {}

void Vertex_stream_separate_ts::release() {
    delete[] p_;
    delete[] ts_;
    delete[] uv_;
}

float3 Vertex_stream_separate_ts::p(uint32_t i) const {
    return float3(p_[i]);
}

Vertex_stream::NT Vertex_stream_separate_ts::nt(uint32_t i) const {
    Quaternion ts = ts_[i];

    if (ts[3] < 0.f) {
        ts[3] = -ts[3];
    }

    float3x3 const tbn = quaternion::create_matrix3x3(ts);

    return {tbn.r[2], tbn.r[0]};
}

float2 Vertex_stream_separate_ts::uv(uint32_t i) const {
    return uv_[i];
}

bool Vertex_stream_separate_ts::bitangent_sign(uint32_t i) const {
    return ts_[i][3] < 0.f;
}

Vertex_stream_separate_compact::Vertex_stream_separate_compact(uint32_t             num_vertices,
                                                               packed_float3 const* p,
                                                               packed_float3 const* n)
    : Vertex_stream(num_vertices), p_(p), n_(n) {}

void Vertex_stream_separate_compact::release() {
    delete[] p_;
    delete[] n_;
}

float3 Vertex_stream_separate_compact::p(uint32_t i) const {
    return float3(p_[i]);
}

Vertex_stream::NT Vertex_stream_separate_compact::nt(uint32_t i) const {
    return {float3(n_[i]), tangent(float3(n_[i]))};
}

float2 Vertex_stream_separate_compact::uv(uint32_t /*i*/) const {
    return float2(0.f);
}

bool Vertex_stream_separate_compact::bitangent_sign(uint32_t /*i*/) const {
    return false;
}

}  // namespace model
