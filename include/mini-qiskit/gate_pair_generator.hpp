#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>

#include "mini-qiskit/common/mathtools.hpp"

namespace mqis
{

/*
    The SingleQubitGatePairGenerator loops over all pairs of computational states which
    differ on bit `qubit_index`, and yields them using the `next()` member function.

    The number of yielded pairs is always 2^(n_qubits - 1).

    Separating the index looping from the simulation code makes it easier to test if the
    correct pairs of indices are being chosen.

    C++20 doesn't support generators :(
*/
class SingleQubitGatePairGenerator
{
public:
    constexpr SingleQubitGatePairGenerator(std::size_t qubit_index, std::size_t n_qubits)
        : i0_max_ {impl_mqis::pow_2_int(qubit_index)}
        , i1_max_ {impl_mqis::pow_2_int(n_qubits - qubit_index - 1)}
    {}

    constexpr auto size() const noexcept -> std::size_t
    {
        return i0_max_ * i1_max_;
    }

    constexpr auto next() noexcept -> std::tuple<std::size_t, std::size_t>
    {
        const auto current_i0 = i0_;
        const auto current_i1 = i1_;

        ++i1_;
        if (i1_ == i1_max_) {
            ++i0_;
            i1_ = 0;
        }

        // indices corresponding to the computational basis states where the [i1]^th digit
        // are 0 and 1, respectively
        const auto state0_index = current_i0 + 2 * current_i1 * i0_max_;
        const auto state1_index = state0_index + i0_max_;

        return {state0_index, state1_index};
    }

private:
    std::size_t i0_max_;
    std::size_t i1_max_;
    std::size_t i0_ {0};
    std::size_t i1_ {0};
};

/*
    The DoubleQubitGatePairIterator loops over all pairs of computational states where
      - in the first state, the qubits at (source_index, target_index) are (1, 0)
      - in the second state, the qubits at (source_index, target_index) are (1, 1)
    and yields them using the `next()` member function.

    The number of yielded pairs is always 2^(n_qubits - 2).

    Separating the index looping from the simulation code makes it easier to test if the
    correct pairs of indices are being chosen.

    C++20 doesn't support generators :(
*/
class DoubleQubitGatePairGenerator
{
public:
    DoubleQubitGatePairGenerator(std::size_t source_index, std::size_t target_index, std::size_t n_qubits)
        : lower_index_ {std::min({source_index, target_index})}
        , upper_index_ {std::max({source_index, target_index})}
        , lower_shift_ {impl_mqis::pow_2_int(lower_index_ + 1)}
        , upper_shift_ {impl_mqis::pow_2_int(upper_index_ + 1)}
        , source_shift_ {impl_mqis::pow_2_int(source_index)}
        , target_shift_ {impl_mqis::pow_2_int(target_index)}
        , i0_max_ {impl_mqis::pow_2_int(lower_index_)}
        , i1_max_ {impl_mqis::pow_2_int(upper_index_ - lower_index_ - 1)}
        , i2_max_ {impl_mqis::pow_2_int(n_qubits - upper_index_ - 1)}
    {}

    constexpr auto size() const noexcept -> std::size_t
    {
        return i0_max_ * i1_max_ * i2_max_;
    }

    constexpr auto next() noexcept -> std::tuple<std::size_t, std::size_t>
    {
        const auto current_i0 = i0_;
        const auto current_i1 = i1_;
        const auto current_i2 = i2_;

        ++i2_;
        if (i2_ == i2_max_) {
            ++i1_;
            i2_ = 0;

            if (i1_ == i1_max_) {
                ++i0_;
                i1_ = 0;
            }
        }

        const auto state0_index = current_i0 + current_i1 * lower_shift_ + current_i2 * upper_shift_ + source_shift_;
        const auto state1_index = state0_index + target_shift_;

        return {state0_index, state1_index};
    }

private:
    std::size_t lower_index_;
    std::size_t upper_index_;
    std::size_t lower_shift_;
    std::size_t upper_shift_;
    std::size_t source_shift_;
    std::size_t target_shift_;
    std::size_t i0_max_;
    std::size_t i1_max_;
    std::size_t i2_max_;
    std::size_t i0_ {0};
    std::size_t i1_ {0};
    std::size_t i2_ {0};
};

}  // namespace mqis
