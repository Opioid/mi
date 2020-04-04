#ifndef SU_CORE_SCENE_SHAPE_VERTEX_HPP
#define SU_CORE_SCENE_SHAPE_VERTEX_HPP

#include "base/math/quaternion.hpp"
#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace model {

struct Vertex {
    packed_float3 p;
    packed_float3 n;
    packed_float3 t;

    float2 uv;

    uint8_t bitangent_sign;

    uint8_t pad[3];

    static size_t unpadded_size();
};

class Vertex_stream {
  public:
    Vertex_stream(uint32_t num_vertices);

    uint32_t num_vertices() const;

    virtual ~Vertex_stream();

    virtual void release() = 0;

    virtual float3 p(uint32_t i) const = 0;

    struct NT {
        float3 n;
        float3 t;
    };

    virtual NT nt(uint32_t i) const = 0;

    virtual float2 uv(uint32_t i) const = 0;

    virtual bool bitangent_sign(uint32_t i) const = 0;

  private:
    uint32_t num_vertices_;
};

class Vertex_stream_interleaved final : public Vertex_stream {
  public:
    Vertex_stream_interleaved(uint32_t num_vertices, Vertex const* vertices);

    void release() final;

    float3 p(uint32_t i) const final;

    NT nt(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    Vertex const* vertices_;
};

class Vertex_stream_separate final : public Vertex_stream {
  public:
    Vertex_stream_separate(uint32_t num_vertices, packed_float3 const* p, packed_float3 const* n,
                           packed_float3 const* t, float2 const* uv, uint8_t const* bts);

    void release() final;

    float3 p(uint32_t i) const final;

    NT nt(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    packed_float3 const* p_;
    packed_float3 const* n_;
    packed_float3 const* t_;
    float2 const*        uv_;
    uint8_t const*       bts_;
};

class Vertex_stream_separate_ts final : public Vertex_stream {
  public:
    Vertex_stream_separate_ts(uint32_t num_vertices, packed_float3 const* p, Quaternion const* ts,
                              float2 const* uv);

    void release() final;

    float3 p(uint32_t i) const final;

    NT nt(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    packed_float3 const* p_;
    Quaternion const*    ts_;
    float2 const*        uv_;
};

class Vertex_stream_separate_compact final : public Vertex_stream {
  public:
    Vertex_stream_separate_compact(uint32_t num_vertices, packed_float3 const* p,
                                   packed_float3 const* n);

    void release() final;

    float3 p(uint32_t i) const final;

    NT nt(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    packed_float3 const* p_;
    packed_float3 const* n_;
};

}  // namespace model

#endif
