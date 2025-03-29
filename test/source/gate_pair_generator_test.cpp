#include <algorithm>
#include <compare>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mini-qiskit/simulation/gate_pair_generator.hpp"

using BasisIndexMap = std::unordered_map<std::string, std::size_t>;

struct IndexPair
{
    std::size_t index0;
    std::size_t index1;

    friend std::ostream& operator<<(std::ostream& os, const IndexPair& pair)
    {
        os << "(" << pair.index0 << ", " << pair.index1 << ")";
        return os;
    }

    friend constexpr auto operator<=>(const IndexPair& left, const IndexPair& right) = default;
};

template <typename Generator>
auto get_generated_index_pairs(Generator& generator) -> std::vector<IndexPair>
{
    auto index_pairs = std::vector<IndexPair> {};
    for (std::size_t i {0}; i < generator.size(); ++i) {
        const auto [state0_index, state1_index] = generator.next();

        index_pairs.push_back({state0_index, state1_index});
    }

    return index_pairs;
}

constexpr auto num_pairs_for_single_qubit_gate(std::size_t n_qubits) -> std::size_t
{
    // The number of yielded pairs is always 2^(n_qubits - 1):
    // - the qubit at `target_index` is fixed for the pair (one must be set to 0, the other to 1)
    // - the remaining qubits can be in either state
    //   - and there are `n_qubits - 1` of them, with 2 possible states per qubit
    return impl_mqis::pow_2_int(n_qubits - 1);
}

constexpr auto num_pairs_for_double_qubit_gate(std::size_t n_qubits) -> std::size_t
{
    // The number of yielded pairs is always 2^(n_qubits - 2):
    // - the qubit at `control_index` is fixed for the pair (always set to 1)
    // - the qubit at `target_index` is fixed for the pair (one must be set to 0, the other to 1)
    // - the remaining qubits can be in either state
    //   - and there are `n_qubits - 2` of them, with 2 possible states per qubit
    return impl_mqis::pow_2_int(n_qubits - 2);
}

TEST_CASE("SingleQubitGatePairGenerator with two qubits")
{
    const auto n_qubits = std::size_t {2};

    // with two qubits, the states are 00, 10, 01, 11
    const auto coeff_map = BasisIndexMap {
        {"00", 0},
        {"10", 1},
        {"01", 2},
        {"11", 3},
    };

    SECTION("loop over qubit at index 0")
    {
        auto generator = impl_mqis::SingleQubitGatePairGenerator {0, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_single_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("00"), coeff_map.at("10")},
             IndexPair {coeff_map.at("01"), coeff_map.at("11")}
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }

    SECTION("loop over qubit at index 1")
    {
        auto generator = impl_mqis::SingleQubitGatePairGenerator {1, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_single_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("00"), coeff_map.at("01")},
             IndexPair {coeff_map.at("10"), coeff_map.at("11")}
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }
}

TEST_CASE("SingleQubitGatePairGenerator with three qubits")
{
    const auto n_qubits = std::size_t {3};

    // with three qubits, the states are 000, 100, 010, 110, 001, 101, 011, 111
    const auto coeff_map = BasisIndexMap {
        {"000", 0},
        {"100", 1},
        {"010", 2},
        {"110", 3},
        {"001", 4},
        {"101", 5},
        {"011", 6},
        {"111", 7},
    };

    SECTION("loop over qubit at index 0")
    {
        auto generator = impl_mqis::SingleQubitGatePairGenerator {0, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_single_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("000"), coeff_map.at("100")},
            IndexPair {coeff_map.at("010"), coeff_map.at("110")},
            IndexPair {coeff_map.at("001"), coeff_map.at("101")},
            IndexPair {coeff_map.at("011"), coeff_map.at("111")}
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }

    SECTION("loop over qubit at index 1")
    {
        auto generator = impl_mqis::SingleQubitGatePairGenerator {1, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_single_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("000"), coeff_map.at("010")},
            IndexPair {coeff_map.at("100"), coeff_map.at("110")},
            IndexPair {coeff_map.at("001"), coeff_map.at("011")},
            IndexPair {coeff_map.at("101"), coeff_map.at("111")}
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }

    SECTION("loop over qubit at index 2")
    {
        auto generator = impl_mqis::SingleQubitGatePairGenerator {2, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_single_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("000"), coeff_map.at("001")},
            IndexPair {coeff_map.at("100"), coeff_map.at("101")},
            IndexPair {coeff_map.at("010"), coeff_map.at("011")},
            IndexPair {coeff_map.at("110"), coeff_map.at("111")}
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }
}

TEST_CASE("DoubleQubitGatePairGenerator with two qubits")
{
    const auto n_qubits = std::size_t {2};

    // with two qubits, the states are 00, 10, 01, 11
    const auto coeff_map = BasisIndexMap {
        {"00", 0},
        {"10", 1},
        {"01", 2},
        {"11", 3},
    };

    SECTION("control is 0, target is 1")
    {
        auto generator = impl_mqis::DoubleQubitGatePairGenerator {0, 1, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_double_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("10"), coeff_map.at("11")}
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }

    SECTION("control is 1, target is 0")
    {
        auto generator = impl_mqis::DoubleQubitGatePairGenerator {1, 0, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_double_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("01"), coeff_map.at("11")}
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }
}

TEST_CASE("DoubleQubitGatePairGenerator with three qubits")
{
    const auto n_qubits = std::size_t {3};

    // with three qubits, the states are 000, 100, 010, 110, 001, 101, 011, 111
    const auto coeff_map = BasisIndexMap {
        {"000", 0},
        {"100", 1},
        {"010", 2},
        {"110", 3},
        {"001", 4},
        {"101", 5},
        {"011", 6},
        {"111", 7},
    };

    SECTION("(control, target) = (0, 1)")
    {
        auto generator = impl_mqis::DoubleQubitGatePairGenerator {0, 1, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_double_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("100"), coeff_map.at("110")},
            IndexPair {coeff_map.at("101"), coeff_map.at("111")},
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }

    SECTION("(control, target) = (1, 0)")
    {
        auto generator = impl_mqis::DoubleQubitGatePairGenerator {1, 0, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_double_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("010"), coeff_map.at("110")},
            IndexPair {coeff_map.at("011"), coeff_map.at("111")},
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }

    SECTION("(control, target) = (0, 2)")
    {
        auto generator = impl_mqis::DoubleQubitGatePairGenerator {0, 2, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_double_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("100"), coeff_map.at("101")},
            IndexPair {coeff_map.at("110"), coeff_map.at("111")},
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }

    SECTION("(control, target) = (1, 2)")
    {
        auto generator = impl_mqis::DoubleQubitGatePairGenerator {1, 2, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_double_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("010"), coeff_map.at("011")},
            IndexPair {coeff_map.at("110"), coeff_map.at("111")},
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }
}

TEST_CASE("DoubleQubitGatePairGenerator with four qubits")
{
    const auto n_qubits = std::size_t {4};

    // with four qubits, the states are
    // 0000, 1000, 0100, 1100, 0010, 1010, 0110, 1110
    // 0001, 1001, 0101, 1101, 0011, 1011, 0111, 1111
    const auto coeff_map = BasisIndexMap {
        {"0000", 0 },
        {"1000", 1 },
        {"0100", 2 },
        {"1100", 3 },
        {"0010", 4 },
        {"1010", 5 },
        {"0110", 6 },
        {"1110", 7 },
        {"0001", 8 },
        {"1001", 9 },
        {"0101", 10},
        {"1101", 11},
        {"0011", 12},
        {"1011", 13},
        {"0111", 14},
        {"1111", 15},
    };

    SECTION("(control, target) = (0, 1)")
    {
        auto generator = impl_mqis::DoubleQubitGatePairGenerator {0, 1, n_qubits};

        REQUIRE(generator.size() == num_pairs_for_double_qubit_gate(n_qubits));

        auto expected = std::vector<IndexPair> {
            IndexPair {coeff_map.at("1000"), coeff_map.at("1100")},
            IndexPair {coeff_map.at("1010"), coeff_map.at("1110")},
            IndexPair {coeff_map.at("1001"), coeff_map.at("1101")},
            IndexPair {coeff_map.at("1011"), coeff_map.at("1111")},
        };

        auto actual = get_generated_index_pairs(generator);

        // there isn't really a guaranteed ordered, AFAIK
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        REQUIRE(expected == actual);
    }
}
