#pragma once

#include <complex>

#include "kettle/common/tolerance.hpp"

namespace ket
{

auto almost_eq(
    const std::complex<double>& left,
    const std::complex<double>& right,
    double tolerance_sq = ket::COMPLEX_ALMOST_EQ_TOLERANCE_SQ
) noexcept -> bool;

auto endian_flip(std::size_t value, std::size_t n_relevant_bits) -> std::size_t;

}  // namespace ket
