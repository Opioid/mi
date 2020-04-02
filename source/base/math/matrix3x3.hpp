#ifndef SU_BASE_MATH_MATRIX3X3_HPP
#define SU_BASE_MATH_MATRIX3X3_HPP

#include "matrix.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

/****************************************************************************
 *
 * Aligned 3x3 float matrix
 *
 ****************************************************************************/

struct Vector4f_a;

struct alignas(16) Matrix3x3f_a {
    Vector3f_a r[3];

    Matrix3x3f_a();

    constexpr Matrix3x3f_a(float m00, float m01, float m02, float m10, float m11, float m12,
                           float m20, float m21, float m22);

    explicit constexpr Matrix3x3f_a(Vector3f_a const& x, Vector3f_a const& y, Vector3f_a const& z);

    static Matrix3x3f_a constexpr identity();
};

}  // namespace math

#endif
