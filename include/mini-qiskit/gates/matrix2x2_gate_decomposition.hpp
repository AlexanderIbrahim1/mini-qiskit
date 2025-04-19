#pragma once

#include <algorithm>
#include <cmath>
#include <compare>
#include <optional>
#include <vector>

#include "mini-qiskit/common/matrix2x2.hpp"
#include "mini-qiskit/common/utils.hpp"
#include "mini-qiskit/gates/primitive_gate.hpp"
#include "mini-qiskit/gates/common_u_gates.hpp"

/*
    This header file contains functions for decomposing a general 2x2 unitary matrix
    to a product of primitive 2x2 quantum gates.
*/

namespace impl_mqis
{

struct PrimitiveGateInfo
{
    mqis::Gate gate;
    std::optional<double> parameter = std::nullopt;
};

/*
    Find the angle of the determinant.
*/
constexpr auto determinant_angle_(const mqis::Matrix2X2& matrix) -> double
{
    const auto det = mqis::determinant(matrix);
    return std::atan2(det.imag(), det.real());
}

/*
    Decompose a 2x2 unitary matrix into one of several primitive 1-qubit unitary gates.

    This function attempts to construct unparameterized gates before paramterized gates. For
    example, if the matrix [1, 0; 0 -1] is passed as an input, this function will attempt
    to decompose it as a Z gate instead of an RZ gate with an angle of (-M_PI/2).
*/
inline auto decomp_to_single_primitive_gate_(
    const mqis::Matrix2X2& unitary,
    double tolerance_sq = impl_mqis::COMPLEX_ALMOST_EQ_TOLERANCE_SQ
) -> std::optional<impl_mqis::PrimitiveGateInfo>
{
    using Info = impl_mqis::PrimitiveGateInfo;
    // NOTES:
    // - the H, X, Y, and Z gates take no arguments, and so we can check them directly
    // - the RX, RY, RZ gates all share the feature that the angle can be recovered
    //   from the arccos of the real component of elem11
    // - the P gate requires information from both the real and imaginary components of elem11
    if (almost_eq(unitary, mqis::h_gate(), tolerance_sq)) {
        return Info {mqis::Gate::H, {}};
    }
    else if (almost_eq(unitary, mqis::x_gate(), tolerance_sq)) {
        return Info {mqis::Gate::X, {}};
    }
    else if (almost_eq(unitary, mqis::y_gate(), tolerance_sq)) {
        return Info {mqis::Gate::Y, {}};
    }
    else if (almost_eq(unitary, mqis::z_gate(), tolerance_sq)) {
        return Info {mqis::Gate::Z, {}};
    }
    if (almost_eq(unitary, mqis::sx_gate(), tolerance_sq)) {
        return Info {mqis::Gate::SX, {}};
    }
    else {
        const auto real_11 = std::clamp(unitary.elem11.real(), -1.0, 1.0);
        const auto imag_11 = std::clamp(unitary.elem11.imag(), -1.0, 1.0);
        const auto theta = std::acos(real_11);
        const auto p_theta = std::atan2(imag_11, real_11);

        if (almost_eq(unitary, mqis::rx_gate(2.0 * theta), tolerance_sq)) {
            return Info {mqis::Gate::RX, 2.0 * theta};
        }
        else if (almost_eq(unitary, mqis::ry_gate(2.0 * theta), tolerance_sq)) {
            return Info {mqis::Gate::RY, 2.0 * theta};
        }
        else if (almost_eq(unitary, mqis::rz_gate(2.0 * theta), tolerance_sq)) {
            return Info {mqis::Gate::RZ, 2.0 * theta};
        }
        else if (almost_eq(unitary, mqis::p_gate(p_theta), tolerance_sq)) {
            return Info {mqis::Gate::P, p_theta};
        }
    }

    return std::nullopt;
}

/*
    The implementation of this decomposition is taken directly from the following file:
        https://github.com/fedimser/quantum_decomp/blob/master/quantum_decomp/src/decompose_2x2.py
    
    The author of the repository if fedimser.
    The repo is published under the MIT license.
*/
inline auto decomp_special_unitary_to_primitive_gates_(
    const mqis::Matrix2X2& unitary,
    double tolerance_sq = impl_mqis::COMPLEX_ALMOST_EQ_TOLERANCE_SQ
) -> std::vector<impl_mqis::PrimitiveGateInfo>
{
    using Info = impl_mqis::PrimitiveGateInfo;

    const auto abs00 = std::clamp(std::abs(unitary.elem00), 0.0, 1.0);

    const auto theta = std::acos(abs00);
    const auto lambda = std::atan2(unitary.elem00.imag(), unitary.elem00.real());
    const auto mu = std::atan2(unitary.elem01.imag(), unitary.elem01.real());

    auto output = std::vector<Info> {};

    if (std::fabs(lambda - mu) > tolerance_sq) {
        output.emplace_back(mqis::Gate::RZ, lambda - mu);
    }

    if (std::fabs(2.0 * theta) > tolerance_sq) {
        output.emplace_back(mqis::Gate::RY, theta);
    }

    if (std::fabs(lambda + mu) > tolerance_sq) {
        output.emplace_back(mqis::Gate::RZ, lambda + mu);
    }

    return output;
}

inline auto decomp_to_primitive_gates_(
    const mqis::Matrix2X2& unitary,
    double tolerance_sq = impl_mqis::COMPLEX_ALMOST_EQ_TOLERANCE_SQ
) -> std::vector<impl_mqis::PrimitiveGateInfo>
{
    const auto primitive = decomp_to_single_primitive_gate_(unitary);
    if (primitive) {
        return {primitive.value()};
    }

    const auto det_angle = determinant_angle_(unitary);

    if (std::fabs(det_angle) < tolerance_sq) {
        return decomp_special_unitary_to_primitive_gates_(unitary, tolerance_sq);
    }
    else {
        const auto left_mat = mqis::p_gate(-det_angle);
        return decomp_special_unitary_to_primitive_gates_(left_mat * unitary, tolerance_sq);
    }
}

}  // namespace impl_mqis
