#include <cmath>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "mini-qiskit/calculations/probabilities.hpp"
#include "mini-qiskit/circuit/circuit.hpp"
#include "mini-qiskit/simulation/simulate.hpp"
#include "mini-qiskit/state/state.hpp"

static constexpr auto RELATIVE_TOL = 1.0e-6;

TEST_CASE("cumulative probabilities")
{
    const auto probabilities = std::vector<double>(4, 0.25);
    const auto cumulative = impl_mqis::calculate_cumulative_sum(probabilities);

    REQUIRE_THAT(cumulative[0], Catch::Matchers::WithinRel(0.25));
    REQUIRE_THAT(cumulative[1], Catch::Matchers::WithinRel(0.50));
    REQUIRE_THAT(cumulative[2], Catch::Matchers::WithinRel(0.75));
    REQUIRE_THAT(cumulative[3], Catch::Matchers::WithinRel(1.00));
}

TEST_CASE("probabilities_raw")
{
    using QSE = mqis::QuantumStateEndian;

    SECTION("computational basis")
    {
        struct InputAndOutput
        {
            mqis::QuantumState input;
            std::vector<double> output;
        };

        // clang-format off
        auto pair = GENERATE(
            InputAndOutput {mqis::QuantumState {"00", QSE::LITTLE}, {1.0, 0.0, 0.0, 0.0}},
            InputAndOutput {mqis::QuantumState {"10", QSE::LITTLE}, {0.0, 1.0, 0.0, 0.0}},
            InputAndOutput {mqis::QuantumState {"01", QSE::LITTLE}, {0.0, 0.0, 1.0, 0.0}},
            InputAndOutput {mqis::QuantumState {"11", QSE::LITTLE}, {0.0, 0.0, 0.0, 1.0}}
        );
        // clang-format on

        const auto output = mqis::calculate_probabilities_raw(pair.input);
        REQUIRE_THAT(output, Catch::Matchers::Approx(pair.output));
    }

    SECTION("one qubit, after H gate")
    {
        auto circuit = mqis::QuantumCircuit {1};
        circuit.add_h_gate(0);

        auto state = mqis::QuantumState {"0"};
        mqis::simulate(circuit, state);

        const auto actual = mqis::calculate_probabilities_raw(state);
        const auto expected = std::vector<double> {0.5, 0.5};

        REQUIRE_THAT(actual, Catch::Matchers::Approx(expected));
    }

    SECTION("one qubit, real and imaginary distribution")
    {
        auto device = std::random_device {};
        auto prng = std::mt19937 {device()};
        auto distrib = std::uniform_real_distribution<double> {-1.0, 1.0};

        for (std::size_t i {0}; i < 10; ++i) {
            const auto real = distrib(prng);
            const auto imag = std::sqrt(1.0 - real * real);

            const auto state = mqis::QuantumState {
                {{real, imag}, {0.0, 0.0}}
            };
            const auto actual = mqis::calculate_probabilities_raw(state);
            const auto expected = std::vector<double> {1.0, 0.0};

            REQUIRE_THAT(actual, Catch::Matchers::Approx(expected));
        }
    }
}

TEST_CASE("probabilities")
{
    using QSE = mqis::QuantumStateEndian;

    SECTION("computational basis")
    {
        struct InputAndOutput
        {
            mqis::QuantumState input;
            std::unordered_map<std::string, double> output;
        };

        // clang-format off
        auto pair = GENERATE(
            InputAndOutput {mqis::QuantumState {"00", QSE::LITTLE}, {{"00", 1.0}, {"10", 0.0}, {"01", 0.0}, {"11", 0.0}}},
            InputAndOutput {mqis::QuantumState {"10", QSE::LITTLE}, {{"00", 0.0}, {"10", 1.0}, {"01", 0.0}, {"11", 0.0}}},
            InputAndOutput {mqis::QuantumState {"01", QSE::LITTLE}, {{"00", 0.0}, {"10", 0.0}, {"01", 1.0}, {"11", 0.0}}},
            InputAndOutput {mqis::QuantumState {"11", QSE::LITTLE}, {{"00", 0.0}, {"10", 0.0}, {"01", 0.0}, {"11", 1.0}}}
        );
        // clang-format on

        const auto output = mqis::calculate_probabilities(pair.input);

        for (const auto& bitstring : {"00", "10", "01", "11"}) {
            const auto expected = pair.output.at(bitstring);
            const auto actual = output.at(bitstring);
            REQUIRE_THAT(actual, Catch::Matchers::WithinAbs(expected, RELATIVE_TOL));
        }
    }

    SECTION("one qubit, after H gate")
    {
        auto circuit = mqis::QuantumCircuit {1};
        circuit.add_h_gate(0);

        auto state = mqis::QuantumState {"0"};
        mqis::simulate(circuit, state);

        const auto actual = mqis::calculate_probabilities(state);
        const auto expected = std::unordered_map<std::string, double> {
            {"0", 0.5},
            {"1", 0.5}
        };

        REQUIRE_THAT(actual.at("0"), Catch::Matchers::WithinAbs(expected.at("0"), RELATIVE_TOL));
        REQUIRE_THAT(actual.at("1"), Catch::Matchers::WithinAbs(expected.at("1"), RELATIVE_TOL));
    }

    SECTION("one qubit, real and imaginary distribution")
    {
        auto device = std::random_device {};
        auto prng = std::mt19937 {device()};
        auto distrib = std::uniform_real_distribution<double> {-1.0, 1.0};

        for (std::size_t i {0}; i < 10; ++i) {
            const auto real = distrib(prng);
            const auto imag = std::sqrt(1.0 - real * real);

            const auto state = mqis::QuantumState {
                {{real, imag}, {0.0, 0.0}}
            };

            const auto actual = mqis::calculate_probabilities(state);
            const auto expected = std::unordered_map<std::string, double> {
                {"0", 1.0},
                {"1", 0.0}
            };

            REQUIRE_THAT(actual.at("0"), Catch::Matchers::WithinAbs(expected.at("0"), RELATIVE_TOL));
            REQUIRE_THAT(actual.at("1"), Catch::Matchers::WithinAbs(expected.at("1"), RELATIVE_TOL));
        }
    }
}
