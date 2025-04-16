#pragma once

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include "mini-qiskit/common/utils.hpp"
#include "mini-qiskit/state/endian.hpp"

namespace impl_mqis
{

constexpr auto endian_flip_(std::size_t value, std::size_t n_relevant_bits) -> std::size_t
{
    auto backward = std::size_t {0};

    for (std::size_t i {0}; i < n_relevant_bits; ++i) {
        backward <<= 1;
        backward |= (value & 1);
        value >>= 1;
    }

    return backward;
}

inline auto is_valid_marginal_bitstring_(const std::string& bitstring) -> bool
{
    const auto is_bitstring_char = [](char bitchar) {
        return bitchar == '0' || bitchar == '1' || bitchar == impl_mqis::MARGINALIZED_QUBIT;
    };

    return std::all_of(bitstring.begin(), bitstring.end(), is_bitstring_char);
}

inline auto is_valid_nonmarginal_bitstring_(const std::string& bitstring) -> bool
{
    const auto is_nonmarginal_bitstring_char = [](char bitchar) {
        return bitchar == '0' || bitchar == '1';
    };

    return std::all_of(bitstring.begin(), bitstring.end(), is_nonmarginal_bitstring_char);
}

inline void check_bitstring_is_valid_nonmarginal_(const std::string& bitstring)
{
    if (!is_valid_nonmarginal_bitstring_(bitstring)) {
        auto err_msg = std::stringstream {};
        err_msg << "Received a bitstring with inputs that aren't ";
        err_msg << "'0' or '1': ";
        err_msg << bitstring << '\n';

        throw std::runtime_error {err_msg.str()};
    }
}

inline void check_bitstring_is_valid_marginal_(const std::string& bitstring)
{
    if (!is_valid_marginal_bitstring_(bitstring)) {
        auto err_msg = std::stringstream {};
        err_msg << "Received a bitstring with inputs that aren't ";
        err_msg << "'0', '1' or '" << impl_mqis::MARGINALIZED_QUBIT << "' : ";
        err_msg << bitstring << '\n';

        throw std::runtime_error {err_msg.str()};
    }
}

}  // namespace impl_mqis
