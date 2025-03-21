#pragma once

#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <unordered_map>

#include "mini-qiskit/circuit.hpp"
#include "mini-qiskit/common/matrix2x2.hpp"
#include "mini-qiskit/common/utils.hpp"
#include "mini-qiskit/gate_pair_generator.hpp"
#include "mini-qiskit/operations.hpp"
#include "mini-qiskit/state.hpp"

namespace impl_mqis
{

template <mqis::Gate GateType>
void simulate_single_qubit_gate(mqis::QuantumState& state, const mqis::GateInfo& info, std::size_t n_qubits)
{
    using Gate = mqis::Gate;

    const auto qubit_index = unpack_single_qubit_gate_index(info);

    auto pair_iterator = SingleQubitGatePairGenerator {qubit_index, n_qubits};

    for (std::size_t i {0}; i < pair_iterator.size(); ++i) {
        const auto [state0_index, state1_index] = pair_iterator.next();

        if constexpr (GateType == Gate::X) {
            swap_states(state, state0_index, state1_index);
        }
        else if constexpr (GateType == Gate::H) {
            superpose_states(state, state0_index, state1_index);
        }
        else if constexpr (GateType == Gate::RX) {
            [[maybe_unused]] const auto [theta, ignore] = unpack_rx_gate(info);
            turn_states(state, state0_index, state1_index, theta);
        }
        else {
            static_assert(
                impl_mqis::always_false<void>::value, "Invalid single qubit gate: must be one of {X, H, RX}."
            );
        }
    }
}

void simulate_single_qubit_gate_general(
    mqis::QuantumState& state,
    const mqis::GateInfo& info,
    std::size_t n_qubits,
    const mqis::Matrix2X2& mat
)
{
    const auto qubit_index = unpack_single_qubit_gate_index(info);
    auto pair_iterator = SingleQubitGatePairGenerator {qubit_index, n_qubits};

    for (std::size_t i {0}; i < pair_iterator.size(); ++i) {
        const auto [state0_index, state1_index] = pair_iterator.next();
        general_gate_transform(state, state0_index, state1_index, mat);
    }
}

template <mqis::Gate GateType>
void simulate_double_qubit_gate(mqis::QuantumState& state, const mqis::GateInfo& info, std::size_t n_qubits)
{
    using Gate = mqis::Gate;

    const auto [source_index, target_index] = unpack_double_qubit_gate_indices(info);

    auto pair_iterator = DoubleQubitGatePairGenerator {source_index, target_index, n_qubits};

    for (std::size_t i {0}; i < pair_iterator.size(); ++i) {
        [[maybe_unused]] const auto [state0_index, state1_index] = pair_iterator.next();

        if constexpr (GateType == Gate::CX) {
            swap_states(state, state0_index, state1_index);
        }
        else if constexpr (GateType == Gate::CRX) {
            [[maybe_unused]] const auto [ignore0, ignore1, theta] = unpack_crx_gate(info);
            turn_states(state, state0_index, state1_index, theta);
        }
        else if constexpr (GateType == Gate::CP) {
            // NOTE: the DoubleQubitGatePairGenerator needs to calculate the `state0_index` before
            // it calculates the `state1_index`, so we're not losing too much in terms of performance
            [[maybe_unused]] const auto [ignore0, ignore1, theta] = unpack_cp_gate(info);
            controlled_phaseturn_state(state, state1_index, theta);
        }
        else {
            static_assert(impl_mqis::always_false<void>::value, "Invalid double qubit gate: must be one of {CX, CRX}");
        }
    }
}

void simulate_double_qubit_gate_general(
    mqis::QuantumState& state,
    const mqis::GateInfo& info,
    std::size_t n_qubits,
    const mqis::Matrix2X2& mat
)
{
    const auto [source_index, target_index] = unpack_double_qubit_gate_indices(info);
    auto pair_iterator = DoubleQubitGatePairGenerator {source_index, target_index, n_qubits};

    for (std::size_t i {0}; i < pair_iterator.size(); ++i) {
        const auto [state0_index, state1_index] = pair_iterator.next();
        general_gate_transform(state, state0_index, state1_index, mat);
    }
}

}  // namespace impl_mqis

namespace mqis
{

inline void simulate(const QuantumCircuit& circuit, QuantumState& state)
{
    if (circuit.n_qubits() != state.n_qubits()) {
        throw std::runtime_error {"Invalid simulation; circuit and state have different number of qubits."};
    }

    auto measured_clbit_to_qubit = std::unordered_map<std::size_t, std::size_t> {};

    for (const auto& gate : circuit) {
        switch (gate.gate) {
            case Gate::M : {
                const auto [qubit_index, bit_index] = impl_mqis::unpack_m_gate(gate);
                measured_clbit_to_qubit[bit_index] = qubit_index;
                break;
            }
            case Gate::X : {
                impl_mqis::simulate_single_qubit_gate<Gate::X>(state, gate, circuit.n_qubits());
                break;
            }
            case Gate::H : {
                impl_mqis::simulate_single_qubit_gate<Gate::H>(state, gate, circuit.n_qubits());
                break;
            }
            case Gate::RX : {
                impl_mqis::simulate_single_qubit_gate<Gate::RX>(state, gate, circuit.n_qubits());
                break;
            }
            case Gate::U : {
                const auto matrix_index = impl_mqis::unpack_matrix_index(gate);
                const auto& matrix = circuit.unitary_gate(matrix_index);
                impl_mqis::simulate_single_qubit_gate_general(state, gate, circuit.n_qubits(), matrix);
                break;
            }
            case Gate::CU : {
                const auto matrix_index = impl_mqis::unpack_matrix_index(gate);
                const auto& matrix = circuit.unitary_gate(matrix_index);
                impl_mqis::simulate_double_qubit_gate_general(state, gate, circuit.n_qubits(), matrix);
                break;
            }
            case Gate::CX : {
                impl_mqis::simulate_double_qubit_gate<Gate::CX>(state, gate, circuit.n_qubits());
                break;
            }
            case Gate::CRX : {
                impl_mqis::simulate_double_qubit_gate<Gate::CRX>(state, gate, circuit.n_qubits());
                break;
            }
            case Gate::CP : {  // replace with default?
                impl_mqis::simulate_double_qubit_gate<Gate::CP>(state, gate, circuit.n_qubits());
                break;
            }
        }
    }
}

}  // namespace mqis
