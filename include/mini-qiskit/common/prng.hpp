#pragma once

#include <concepts>
#include <optional>
#include <random>

/*
    This header file contains functions related to random number generation and sampling.
*/

namespace impl_mqis
{

/*
    A concept to mimic 'std::discrete_distribution'; namely, the type must produces random
    integers on the interval '[​0​, n)', where the probability of each individual integer 'i' is
    defined as 'wi/S', where 'wi' is the weight of the 'i'th integer and 'S' is the sum of all
    'n' weights. 

    This concept is useful for unit testing, where we might want to create rigged distributions
    to produce certain outcomes.
*/
template <typename T>
concept DiscreteDistribution = requires(T t, std::mt19937& prng)
{
    typename T::result_type;
    T {{0.0, 1.0}};
    { t(prng) } -> std::integral;
};

inline auto get_prng_(std::optional<int> seed) -> std::mt19937
{
    if (seed) {
        const auto seed_val = static_cast<std::mt19937::result_type>(seed.value());
        return std::mt19937 {seed_val};
    }
    else {
        auto device = std::random_device {};
        return std::mt19937 {device()};
    }
}

}  // namespace impl_mqis
