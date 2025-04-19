#pragma once

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>

#include "mini-qiskit/circuit/circuit.hpp"
#include "mini-qiskit/common/utils.hpp"
#include "mini-qiskit/gates/common_u_gates.hpp"
#include "mini-qiskit/gates/multiplicity_controlled_u_gate.hpp"
#include "mini-qiskit/gates/primitive_gate.hpp"
#include "mini-qiskit/gates/toffoli.hpp"


namespace impl_mqis
{

template <impl_mqis::QubitIndices Container = impl_mqis::QubitIndicesIList>
void check_all_indices_are_unique_(const Container& container)
{
    auto seen = std::unordered_set<std::size_t> {};
    for (auto index : container) {
        if (seen.contains(index)) {
            throw std::runtime_error {"The new qubit indices for the controlled circuit must be unique."};
        }

        seen.insert(index);
    }
}

template <impl_mqis::QubitIndices Container = impl_mqis::QubitIndicesIList>
void check_valid_number_of_mapped_indices_(const Container& container, const mqis::QuantumCircuit& circuit)
{
    const auto size = impl_mqis::get_container_size(container);

    if (size != circuit.n_qubits()) {
        throw std::runtime_error {
            "The number of proposed new qubit indices does not match the number of qubits in the subcircuit."
        };
    }
}

template <impl_mqis::QubitIndices Container = impl_mqis::QubitIndicesIList>
void check_control_qubit_is_not_a_mapped_qubit_(const Container& container, std::size_t control_qubit)
{
    const auto is_control_qubit = [&](std::size_t i) { return i == control_qubit; };
    const auto it = std::find_if(container.begin(), container.end(), is_control_qubit);

    if (it != container.end()) {
        throw std::runtime_error {"The control qubit cannot be one of the mapped qubit indices"};
    }
}

template <impl_mqis::QubitIndices Container = impl_mqis::QubitIndicesIList>
void check_no_overlap_between_control_qubits_and_mapped_qubits_(const Container& mapped_qubits, const Container& control_qubits)
{
    auto control_qubit_set = std::unordered_set<std::size_t> {control_qubits.begin(), control_qubits.end()};

    for (auto mapped_qubit : mapped_qubits) {
        if (control_qubit_set.contains(mapped_qubit)) {
            throw std::runtime_error {"The control qubit indices cannot overlap with the mapped qubit indices"};
        }
    }
}

template <impl_mqis::QubitIndices Container = impl_mqis::QubitIndicesIList>
void check_new_indices_fit_onto_new_circuit_(const Container& container, std::size_t control_qubit, std::size_t n_qubits_on_new_circuit)
{
    // the additional '1' comes from the control qubit
    const auto n_minimum_new_indices = impl_mqis::get_container_size(container) + 1;
    if (n_minimum_new_indices > n_qubits_on_new_circuit) {
        throw std::runtime_error {"The mapped qubits will not fit onto the new circuit"};
    }

    const auto is_outside_new_qubits = [&](std::size_t i) { return i >= n_qubits_on_new_circuit; };

    if (is_outside_new_qubits(control_qubit)) {
        throw std::runtime_error {"The control qubit is outside the range of qubits on the new circuit."};
    }

    if (std::any_of(container.begin(), container.end(), is_outside_new_qubits)) {
        throw std::runtime_error {"A mapped qubit was found to be outside the range of qubits on the new circuit."};
    }
}

template <impl_mqis::QubitIndices Container = impl_mqis::QubitIndicesIList>
void check_new_indices_fit_onto_new_circuit_(const Container& mapped_qubits, const Container& control_qubits, std::size_t n_qubits_on_new_circuit)
{
    const auto n_mapped_indices = impl_mqis::get_container_size(mapped_qubits);
    const auto n_control_indices = impl_mqis::get_container_size(control_qubits);
    const auto n_minimum_new_indices = n_mapped_indices + n_control_indices;

    if (n_minimum_new_indices > n_qubits_on_new_circuit) {
        throw std::runtime_error {"The mapped qubits will not fit onto the new circuit"};
    }

    const auto is_outside_new_qubits = [&](std::size_t i) { return i >= n_qubits_on_new_circuit; };

    if (std::any_of(control_qubits.begin(), control_qubits.end(), is_outside_new_qubits)) {
        throw std::runtime_error {"The control qubit is outside the range of qubits on the new circuit."};
    }

    if (std::any_of(mapped_qubits.begin(), mapped_qubits.end(), is_outside_new_qubits)) {
        throw std::runtime_error {"A mapped qubit was found to be outside the range of qubits on the new circuit."};
    }
}

inline void make_one_target_gate_controlled(
    mqis::QuantumCircuit& circuit,
    mqis::Gate gate,
    std::size_t control,
    std::size_t target
)
{
    using G = mqis::Gate;

    switch (gate)
    {
        case G::H : {
            circuit.add_ch_gate(control, target);
            break;
        }
        case G::X : {
            circuit.add_cx_gate(control, target);
            break;
        }
        case G::Y : {
            circuit.add_cy_gate(control, target);
            break;
        }
        case G::Z : {
            circuit.add_cz_gate(control, target);
            break;
        }
        case G::SX : {
            circuit.add_csx_gate(control, target);
            break;
        }
        default : {
            throw std::runtime_error {"DEV ERROR: impossible branch\n"};
        }
    }
}

inline void make_one_target_one_angle_gate_controlled(
    mqis::QuantumCircuit& circuit,
    mqis::Gate gate,
    std::size_t control,
    std::size_t target,
    double angle
)
{
    using G = mqis::Gate;

    switch (gate)
    {
        case G::RX : {
            circuit.add_crx_gate(control, target, angle);
            break;
        }
        case G::RY : {
            circuit.add_cry_gate(control, target, angle);
            break;
        }
        case G::RZ : {
            circuit.add_crz_gate(control, target, angle);
            break;
        }
        case G::P : {
            circuit.add_cp_gate(control, target, angle);
            break;
        }
        default : {
            throw std::runtime_error {"DEV ERROR: impossible branch\n"};
        }
    }
}

}  // namespace impl_mqis

namespace mqis
{

template <impl_mqis::QubitIndices Container = impl_mqis::QubitIndicesIList>
inline auto make_controlled_circuit(
    const mqis::QuantumCircuit& subcircuit,
    std::size_t n_new_qubits,
    std::size_t control,
    const Container& mapped_qubits
) -> mqis::QuantumCircuit
{
    namespace gid = impl_mqis::gate_id;

    impl_mqis::check_valid_number_of_mapped_indices_(mapped_qubits, subcircuit);
    impl_mqis::check_all_indices_are_unique_(mapped_qubits);
    impl_mqis::check_all_indices_are_unique_({control});
    impl_mqis::check_no_overlap_between_control_qubits_and_mapped_qubits_(mapped_qubits, {control});
    impl_mqis::check_new_indices_fit_onto_new_circuit_(mapped_qubits, {control}, n_new_qubits);

    auto new_circuit = mqis::QuantumCircuit {n_new_qubits};

    for (const auto& gate_info : subcircuit) {
        if (gid::is_one_target_transform_gate(gate_info.gate)) {
            const auto original_target = impl_mqis::unpack_one_target_gate(gate_info);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            impl_mqis::make_one_target_gate_controlled(new_circuit, gate_info.gate, control, new_target);
        }
        else if (gid::is_one_target_one_angle_transform_gate(gate_info.gate))
        {
            const auto [original_target, angle] = impl_mqis::unpack_one_target_one_angle_gate(gate_info);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            impl_mqis::make_one_target_one_angle_gate_controlled(new_circuit, gate_info.gate, control, new_target, angle);
        }
        else if (gid::is_one_control_one_target_transform_gate(gate_info.gate)) {
            const auto [original_control, original_target] = impl_mqis::unpack_one_control_one_target_gate(gate_info);
            const auto new_control = impl_mqis::get_container_index(mapped_qubits, original_control);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto matrix = non_angle_gate(gate_info.gate);
            apply_doubly_controlled_gate(new_circuit, matrix, {control, new_control}, new_target);
        }
        else if (gid::is_one_control_one_target_one_angle_transform_gate(gate_info.gate)) {
            const auto [original_control, original_target, angle] = impl_mqis::unpack_one_control_one_target_one_angle_gate(gate_info);
            const auto new_control = impl_mqis::get_container_index(mapped_qubits, original_control);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto matrix = angle_gate(gate_info.gate, angle);
            apply_doubly_controlled_gate(new_circuit, matrix, {control, new_control}, new_target);
        }
        else if (gate_info.gate == Gate::U) {
            const auto [original_target, original_gate_index] = impl_mqis::unpack_u_gate(gate_info);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto& matrix = subcircuit.unitary_gate(original_gate_index);
            new_circuit.add_cu_gate(matrix, control, new_target);
        }
        else if (gate_info.gate == Gate::CU) {
            const auto [original_control, original_target, original_gate_index] = impl_mqis::unpack_cu_gate(gate_info);
            const auto new_control = impl_mqis::get_container_index(mapped_qubits, original_control);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto& matrix = subcircuit.unitary_gate(original_gate_index);
            apply_doubly_controlled_gate(new_circuit, matrix, {control, new_control}, new_target);
        }
        else if (gate_info.gate == Gate::M) {
            throw std::runtime_error {"Cannot make a measurement gate controlled.\n"};
        }
        else {
            throw std::runtime_error {"UNREACHABLE: dev error, invalid gate found when making controlled circuit.\n"};
        }
    }

    return new_circuit;
}

template <impl_mqis::QubitIndices Container = impl_mqis::QubitIndicesIList>
inline auto make_multiplicity_controlled_circuit(
    const mqis::QuantumCircuit& subcircuit,
    std::size_t n_new_qubits,
    const Container& control_qubits,
    const Container& mapped_qubits
) -> mqis::QuantumCircuit
{
    if (impl_mqis::get_container_size(control_qubits) == 1) {
        const auto control = impl_mqis::get_container_index(control_qubits, 0);
        make_controlled_circuit(subcircuit, n_new_qubits, control, mapped_qubits);
    }

    namespace gid = impl_mqis::gate_id;

    impl_mqis::check_valid_number_of_mapped_indices_(mapped_qubits, subcircuit);
    impl_mqis::check_all_indices_are_unique_(mapped_qubits);
    impl_mqis::check_all_indices_are_unique_(control_qubits);
    impl_mqis::check_no_overlap_between_control_qubits_and_mapped_qubits_(mapped_qubits, control_qubits);
    impl_mqis::check_new_indices_fit_onto_new_circuit_(mapped_qubits, control_qubits, n_new_qubits);

    auto new_circuit = QuantumCircuit {n_new_qubits};

    for (const auto& gate_info : subcircuit) {
        if (gid::is_one_target_transform_gate(gate_info.gate)) {
            const auto original_target = impl_mqis::unpack_one_target_gate(gate_info);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto matrix = non_angle_gate(gate_info.gate);
            apply_multiplicity_controlled_u_gate(new_circuit, matrix, new_target, control_qubits);
        }
        else if (gid::is_one_target_one_angle_transform_gate(gate_info.gate))
        {
            const auto [original_target, angle] = impl_mqis::unpack_one_target_one_angle_gate(gate_info);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto matrix = angle_gate(gate_info.gate, angle);
            apply_multiplicity_controlled_u_gate(new_circuit, matrix, new_target, control_qubits);
        }
        else if (gid::is_one_control_one_target_transform_gate(gate_info.gate)) {
            const auto [original_control, original_target] = impl_mqis::unpack_one_control_one_target_gate(gate_info);
            const auto new_control = impl_mqis::get_container_index(mapped_qubits, original_control);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto new_controls = impl_mqis::extend_container_to_vector(control_qubits, {new_control});
            const auto matrix = non_angle_gate(gate_info.gate);
            apply_multiplicity_controlled_u_gate(new_circuit, matrix, new_target, new_controls);
        }
        else if (gid::is_one_control_one_target_one_angle_transform_gate(gate_info.gate)) {
            const auto [original_control, original_target, angle] = impl_mqis::unpack_one_control_one_target_one_angle_gate(gate_info);
            const auto new_control = impl_mqis::get_container_index(mapped_qubits, original_control);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto new_controls = impl_mqis::extend_container_to_vector(control_qubits, {new_control});
            const auto matrix = angle_gate(gate_info.gate, angle);
            apply_multiplicity_controlled_u_gate(new_circuit, matrix, new_target, new_controls);
        }
        else if (gate_info.gate == Gate::U) {
            const auto [original_target, original_gate_index] = impl_mqis::unpack_u_gate(gate_info);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto& matrix = subcircuit.unitary_gate(original_gate_index);
            apply_multiplicity_controlled_u_gate(new_circuit, matrix, new_target, control_qubits);
        }
        else if (gate_info.gate == Gate::CU) {
            const auto [original_control, original_target, original_gate_index] = impl_mqis::unpack_cu_gate(gate_info);
            const auto new_control = impl_mqis::get_container_index(mapped_qubits, original_control);
            const auto new_target = impl_mqis::get_container_index(mapped_qubits, original_target);
            const auto new_controls = impl_mqis::extend_container_to_vector(control_qubits, {new_control});
            const auto& matrix = subcircuit.unitary_gate(original_gate_index);
            apply_multiplicity_controlled_u_gate(new_circuit, matrix, new_target, new_controls);
        }
        else if (gate_info.gate == Gate::M) {
            throw std::runtime_error {"Cannot make a measurement gate controlled.\n"};
        }
        else {
            throw std::runtime_error {"UNREACHABLE: dev error, invalid gate found when making controlled circuit.\n"};
        }
    }

    return new_circuit;
}

}  // namespace mqis
