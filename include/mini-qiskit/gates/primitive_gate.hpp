#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>

namespace mqis
{

enum class Gate
{
    H,
    X,
    Y,
    Z,
    SX,
    RX,
    RY,
    RZ,
    P,
    CH,
    CX,
    CY,
    CZ,
    CSX,
    CRX,
    CRY,
    CRZ,
    CP,
    U,
    CU,
    M,
    CONTROL
};

/*
    Each gate in the reference specification can have either 1, 2, or 3 arguments. This implementation
    forces the type that carries the information for each gate to hold enough data for the largest
    possible number of arguments (3 in this case).

    This wastes a fair bit of memory.

    A more memory-considerate implementation might treat the information needed for each gate simply
    as a collection of bytes, and use an opcode to determine how many arguments are required.

    But this implementation is not concerned with that.
*/
struct GateInfo
{
    Gate gate;
    std::size_t arg0;
    std::size_t arg1;
    double arg2;
    std::size_t arg3;
};

}  // namespace mqis


namespace impl_mqis::gate_id
{

constexpr auto is_one_target_transform_gate(mqis::Gate gate) -> bool
{
    using G = mqis::Gate;
    return gate == G::H || gate == G::X || gate == G::Y || gate == G::Z || gate == G::SX;
}

constexpr auto is_one_target_one_angle_transform_gate(mqis::Gate gate) -> bool
{
    using G = mqis::Gate;
    return gate == G::RX || gate == G::RY || gate == G::RZ || gate == G::P;
}

constexpr auto is_one_control_one_target_transform_gate(mqis::Gate gate) -> bool
{
    using G = mqis::Gate;
    return gate == G::CH || gate == G::CX || gate == G::CY || gate == G::CZ || gate == G::CSX;
}

constexpr auto is_one_control_one_target_one_angle_transform_gate(mqis::Gate gate) -> bool
{
    using G = mqis::Gate;
    return gate == G::CRX || gate == G::CRY || gate == G::CRZ || gate == G::CP;
}

constexpr auto is_single_qubit_transform_gate(mqis::Gate gate) -> bool
{
    using G = mqis::Gate;
    return is_one_target_transform_gate(gate) || is_one_target_one_angle_transform_gate(gate) || gate == G::U;
}

constexpr auto is_double_qubit_transform_gate(mqis::Gate gate) -> bool
{
    using G = mqis::Gate;
    return is_one_control_one_target_one_angle_transform_gate(gate) \
        || is_one_control_one_target_transform_gate(gate) \
        || gate == G::CU;
}

constexpr auto is_non_angle_transform_gate(mqis::Gate gate) -> bool
{
    return is_one_target_transform_gate(gate) || is_one_control_one_target_transform_gate(gate);
}

constexpr auto is_angle_transform_gate(mqis::Gate gate) -> bool
{
    return is_one_target_one_angle_transform_gate(gate) || is_one_control_one_target_one_angle_transform_gate(gate);
}

}  // namespace impl_mqis::gate_id


namespace impl_mqis
{
/*
    Parameters indicating to the developer that a given gate does not use a certain data member in
    a mqis::GateInfo instance.
*/
constexpr static auto DUMMY_ARG0 = std::size_t {0};
constexpr static auto DUMMY_ARG1 = std::size_t {0};
constexpr static auto DUMMY_ARG2 = double {0.0};
constexpr static auto DUMMY_ARG3 = std::size_t {0};

template <mqis::Gate GateKind>
constexpr auto create_one_target_gate(std::size_t target_index) -> mqis::GateInfo
{
    static_assert(gate_id::is_one_target_transform_gate(GateKind));
    return {GateKind, target_index, DUMMY_ARG1, DUMMY_ARG2, DUMMY_ARG3};
}

constexpr auto unpack_one_target_gate(const mqis::GateInfo& info) -> std::size_t
{
    return info.arg0;  // target_index
}

template <mqis::Gate GateKind>
constexpr auto create_one_target_one_angle_gate(double theta, std::size_t target_index) -> mqis::GateInfo
{
    static_assert(gate_id::is_one_target_one_angle_transform_gate(GateKind));
    return {GateKind, target_index, DUMMY_ARG1, theta, DUMMY_ARG3};
}

constexpr auto unpack_one_target_one_angle_gate(const mqis::GateInfo& info) -> std::tuple<std::size_t, double>
{
    return {info.arg0, info.arg2};  // target_index, angle
}

template <mqis::Gate GateKind>
constexpr auto create_one_control_one_target_one_angle_gate(std::size_t control_index, std::size_t target_index, double theta) -> mqis::GateInfo
{
    static_assert(gate_id::is_one_control_one_target_one_angle_transform_gate(GateKind));
    return {GateKind, control_index, target_index, theta, DUMMY_ARG3};
}

constexpr auto unpack_one_control_one_target_one_angle_gate(const mqis::GateInfo& info) -> std::tuple<std::size_t, std::size_t, double>
{
    return {info.arg0, info.arg1, info.arg2};  // control_index, target_index, angle
}

template <mqis::Gate GateKind>
constexpr auto create_one_control_one_target_gate(std::size_t control_index, std::size_t target_index) -> mqis::GateInfo
{
    static_assert(gate_id::is_one_control_one_target_transform_gate(GateKind));
    return {GateKind, control_index, target_index, DUMMY_ARG2, DUMMY_ARG3};
}

constexpr auto unpack_one_control_one_target_gate(const mqis::GateInfo& info) -> std::tuple<std::size_t, std::size_t>
{
    return {info.arg0, info.arg1};  // control_index, target_index
}

/* Apply the U-gate, with the 2x2 matrix identified by `matrix_index` to the qubit at index `target_index` */
constexpr auto create_u_gate(std::size_t target_index, std::size_t matrix_index) -> mqis::GateInfo
{
    return {mqis::Gate::U, target_index, DUMMY_ARG1, DUMMY_ARG2, matrix_index};
}

/* Parse the relevant information for the U-gate */
constexpr auto unpack_u_gate(const mqis::GateInfo& info) -> std::tuple<std::size_t, std::size_t>
{
    return {info.arg0, info.arg3};  // target_index, matrix_index
}

/* Apply CU-gate, with the 2x2 matrix identified by `matrix_index` to qubits at the `control_index` and `target_index` */
constexpr auto create_cu_gate(std::size_t control_index, std::size_t target_index, std::size_t matrix_index)
    -> mqis::GateInfo
{
    return {mqis::Gate::CU, control_index, target_index, DUMMY_ARG2, matrix_index};
}

/* Parse the relevant information for the CU-gate */
constexpr auto unpack_cu_gate(const mqis::GateInfo& info) -> std::tuple<std::size_t, std::size_t, std::size_t>
{
    return {info.arg0, info.arg1, info.arg3};  // control index, target index, matrix index
}

/* Apply a measurement gate to a given qubit and bit */
constexpr auto create_m_gate(std::size_t qubit_index, std::size_t bit_index) -> mqis::GateInfo
{
    return {mqis::Gate::M, qubit_index, bit_index, DUMMY_ARG2, DUMMY_ARG3};
}

/* Parse the relevant information for the M-gate */
constexpr auto unpack_m_gate(const mqis::GateInfo& info) -> std::tuple<std::size_t, std::size_t>
{
    return {info.arg0, info.arg1};  // qubit index, bit index
}

constexpr auto unpack_single_qubit_gate_index(const mqis::GateInfo& info) -> std::size_t
{
    return info.arg0;  // target_index
}

constexpr auto unpack_double_qubit_gate_indices(const mqis::GateInfo& info) -> std::tuple<std::size_t, std::size_t>
{
    return {info.arg0, info.arg1};  // control_index, target_index
}

constexpr auto unpack_gate_angle(const mqis::GateInfo& info) -> double
{
    return info.arg2;  // angle
}

constexpr auto unpack_gate_matrix_index(const mqis::GateInfo& info) -> std::size_t
{
    return info.arg3;  // matrix_index
}

}  // namespace impl_mqis


namespace impl_mqis::control
{

constexpr static auto IF_STMT = std::size_t {0};
constexpr static auto IF_ELSE_STMT = std::size_t {1};
constexpr static auto REPEAT_STMT = std::size_t {2};
constexpr static auto WHILE_LOOP_STMT = std::size_t {3};

constexpr auto unpack_control_flow_kind(const mqis::GateInfo& info) -> std::size_t
{
    return info.arg3;
}

constexpr auto unpack_control_flow_index(const mqis::GateInfo& info) -> std::size_t
{
    return info.arg0;
}

constexpr auto create_control_flow_gate(std::size_t instruction_index, std::size_t control_flow_kind) -> mqis::GateInfo
{
    return {mqis::Gate::CONTROL, instruction_index, DUMMY_ARG1, DUMMY_ARG2, control_flow_kind};
}

}  // namespace impl_mqis::control

