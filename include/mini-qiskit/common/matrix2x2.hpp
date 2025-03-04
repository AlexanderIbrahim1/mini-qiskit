#pragma once

#include <cmath>
#include <complex>

#include "mini-qiskit/common/mathtools.hpp"

namespace mqis
{

struct Matrix2X2
{
    std::complex<double> elem00;  // top-left
    std::complex<double> elem01;  // top-right
    std::complex<double> elem10;  // bottom-left
    std::complex<double> elem11;  // bottom-right

    constexpr auto operator*=(const Matrix2X2& other) noexcept -> Matrix2X2
    {
        const auto new00 = elem00 * other.elem00 + elem01 * other.elem10;
        const auto new01 = elem00 * other.elem01 + elem01 * other.elem11;
        const auto new10 = elem10 * other.elem00 + elem11 * other.elem10;
        const auto new11 = elem10 * other.elem01 + elem11 * other.elem11;

        elem00 = new00;
        elem01 = new01;
        elem10 = new10;
        elem11 = new11;

        return *this;
    }

    constexpr auto operator+=(const Matrix2X2& other) noexcept -> Matrix2X2
    {
        const auto new00 = elem00 + other.elem00;
        const auto new01 = elem01 + other.elem01;
        const auto new10 = elem10 + other.elem10;
        const auto new11 = elem11 + other.elem11;

        elem00 = new00;
        elem01 = new01;
        elem10 = new10;
        elem11 = new11;

        return *this;
    }
};

constexpr auto operator*(Matrix2X2 lhs, const Matrix2X2& rhs) noexcept -> Matrix2X2
{
    lhs *= rhs;
    return lhs;
}

constexpr auto operator+(Matrix2X2 lhs, const Matrix2X2& rhs) noexcept -> Matrix2X2
{
    lhs += rhs;
    return lhs;
}

inline auto matrix_square_root(const Matrix2X2& mat) -> Matrix2X2
{
    // uses the following: https://en.wikipedia.org/wiki/Square_root_of_a_2_by_2_matrix#A_general_formula
    // we use the solution with the positive roots of s and t
    const auto tau = mat.elem00 + mat.elem11;
    const auto delta = mat.elem00 * mat.elem11 - mat.elem01 * mat.elem10;

    const auto s = std::sqrt(delta);
    const auto t = std::sqrt(tau + 2.0 * s);

    const auto new00 = (mat.elem00 + s) / t;
    const auto new01 = mat.elem01 / t;
    const auto new10 = mat.elem10 / t;
    const auto new11 = (mat.elem11 + s) / t;

    return {new00, new01, new10, new11};
}

constexpr auto conjugate_transpose(const Matrix2X2& mat) -> Matrix2X2
{
    const auto new00 = std::conj(mat.elem00);
    const auto new01 = std::conj(mat.elem10);
    const auto new10 = std::conj(mat.elem01);
    const auto new11 = std::conj(mat.elem11);

    return {new00, new01, new10, new11};
}

constexpr auto almost_eq(
    const Matrix2X2& left,
    const Matrix2X2& right,
    double tolerance_sq = impl_mqis::COMPLEX_ALMOST_EQ_TOLERANCE_EQ
) noexcept -> bool
{
    // clang-format off
    return \
        almost_eq(left.elem00, right.elem00, tolerance_sq) && \
        almost_eq(left.elem10, right.elem10, tolerance_sq) && \
        almost_eq(left.elem01, right.elem01, tolerance_sq) && \
        almost_eq(left.elem11, right.elem11, tolerance_sq);
    // clang-format on
}

}  // namespace mqis
