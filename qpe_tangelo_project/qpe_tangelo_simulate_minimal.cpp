#include <filesystem>
#include <format>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <mini-qiskit/mini-qiskit.hpp>

/*
    Perform QPE for the N = 2 and N = 3 gates for the rotor paper, using the minimal
    output files for the gates of the circuit.
*/

static constexpr auto N_UNITARY_QUBITS_TWO_ROTOR = std::size_t {6};
static constexpr auto N_UNITARY_QUBITS_THREE_ROTOR = std::size_t {9};
static constexpr auto RUN_FROM_START_KEY = int {-1};

struct CommandLineArguments
{
    CommandLineArguments(int argc, char** argv)
    {
        if (argc != 8) {
            throw std::runtime_error {
                "./a.out n_ancilla_qubits n_rotors n_trotter_steps abs_gate_dirpath subcircuit_filename abs_output_dirpath i_continue\n"
            };
        }

        const auto arguments = std::vector<std::string> (argv + 1, argv + argc);

        n_ancilla_qubits = std::stoul(arguments[0]);
        const auto n_rotors = std::stoul(arguments[1]);
        n_trotter_steps = std::stoul(arguments[2]);
        abs_circuits_dirpath = std::filesystem::path {arguments[3]};
        subcircuit_filename = arguments[4];
        abs_output_dirpath = std::filesystem::path {arguments[5]};
        i_continue = std::stoi(arguments[6]);

        if (i_continue <= -2) {
            throw std::runtime_error {
                "'i_continue' must be -1 (for running from start) or a non-negative integer"
            };
        }

        if (n_rotors == 2) {
            n_unitary_qubits = N_UNITARY_QUBITS_TWO_ROTOR;
        }
        else if (n_rotors == 3) {
            n_unitary_qubits = N_UNITARY_QUBITS_THREE_ROTOR;
        }
        else {
            throw std::runtime_error {
                "Invalid number of rotors passed; allowed values are '2' and '3'\n"
            };
        }
    }

    std::size_t n_ancilla_qubits;
    std::size_t n_unitary_qubits;
    std::size_t n_trotter_steps;
    std::filesystem::path abs_circuits_dirpath;
    std::string subcircuit_filename;
    std::filesystem::path abs_output_dirpath;
    int i_continue;
};

void simulate_subcircuit(
    const std::filesystem::path& circuit_filepath,
    mqis::QuantumState& statevector,
    std::size_t n_total_qubits
)
{
    const auto circuit = mqis::read_tangelo_circuit(n_total_qubits, circuit_filepath, 0);
    mqis::simulate(circuit, statevector);
}

auto statevector_filename(int i) -> std::string
{
    return std::format("statevector.dat{}", i);
}

void simulate_unitary(
    const CommandLineArguments& args,
    mqis::QuantumState& statevector,
    std::size_t i_control,
    int& count
)
{
    auto n_powers = 1ul << i_control;
    const auto n_total_qubits = args.n_ancilla_qubits + args.n_unitary_qubits;

    const auto circuit_filepath = [&]() {
        auto output = std::stringstream {};
        output << args.subcircuit_filename << i_control;

        return args.abs_circuits_dirpath / output.str();
    }();

    const auto circuit = mqis::read_tangelo_circuit(n_total_qubits, circuit_filepath, 0);

    for (std::size_t i {0}; i < n_powers; ++i) {
        if (args.i_continue != RUN_FROM_START_KEY && count <= args.i_continue) {
            ++count;
            continue;
        }

        for (std::size_t i_trotter_ {0}; i_trotter_ < args.n_trotter_steps; ++i_trotter_) {
            mqis::simulate(circuit, statevector);
        }

        mqis::save_statevector(args.abs_output_dirpath / statevector_filename(count), statevector);
        ++count;
    }
}

auto main(int argc, char** argv) -> int
{
    const auto args = [&]() {
        try {
            return CommandLineArguments {argc, argv};
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }();

    const auto n_total_qubits = args.n_ancilla_qubits + args.n_unitary_qubits;

    auto statevector = [&]() {
        if (args.i_continue == RUN_FROM_START_KEY) {
            return mqis::QuantumState {n_total_qubits};
        } else {
            return mqis::load_statevector(args.abs_output_dirpath / statevector_filename(args.i_continue));
        }
    }();

    // simulate the initial circuit
    if (args.i_continue == RUN_FROM_START_KEY) {
        simulate_subcircuit(args.abs_circuits_dirpath / "initial_circuit.dat", statevector, n_total_qubits);
        simulate_subcircuit(args.abs_circuits_dirpath / "qft_circuit.dat", statevector, n_total_qubits);
    }

    auto count = int {0};
    for (std::size_t i_control {0}; i_control < args.n_ancilla_qubits; ++i_control) {
        simulate_unitary(args, statevector, i_control, count);
    }

    simulate_subcircuit(args.abs_circuits_dirpath / "iqft_circuit.dat", statevector, n_total_qubits);

    mqis::save_statevector(args.abs_output_dirpath / statevector_filename(count), statevector);

    return 0;
}

