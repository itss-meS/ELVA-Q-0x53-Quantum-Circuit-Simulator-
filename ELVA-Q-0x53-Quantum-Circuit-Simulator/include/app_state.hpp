#pragma once
#include "circuit.hpp"
#include "statevector.hpp"
#include <random>
#include <deque>
#include <map>
#include <string>

enum class SelectedGate { None, H, X, Y, Z, S, T, Rx, Ry, Rz, CNOT, CZ, SWAP, Toffoli, Measure };

struct LogEntry {
    std::string msg;
    float r, g, b;
};

struct AppState {
    int n_qubits = 3;
    Circuit circuit;
    StateVector statevec{3};
    RunResult last_result;
    std::mt19937 rng{std::random_device{}()};

    SelectedGate selected_gate = SelectedGate::None;
    int selected_qubit_a = 0;
    int selected_qubit_b = 1;
    int selected_qubit_c = 2;

    float rx_angle = 0.0f;
    float ry_angle = 0.0f;
    float rz_angle = 0.0f;

    int bloch_qubit = 0;
    int shot_count = 1024;
    std::map<int, int> shot_outcomes;
    bool shots_ready = false;

    std::deque<LogEntry> logs;

    void log(const std::string& msg, float r=0.0f, float g=0.85f, float b=1.0f) {
        logs.push_back({msg, r, g, b});
        if(logs.size() > 100) logs.pop_front();
    }

    void reset(int new_n_qubits) {
        n_qubits = new_n_qubits;
        statevec = StateVector(n_qubits);
        circuit.clear();
        shot_outcomes.clear();
        shots_ready = false;
        log("Reset circuit to " + std::to_string(n_qubits) + " qubits", 0.3f, 1.0f, 0.5f);
    }

    void run_circuit() {
        statevec = StateVector(n_qubits);
        last_result = circuit.run(statevec, rng);
        shot_outcomes.clear();
        shots_ready = false;
        log("Executed circuit with " + std::to_string(circuit.ops.size()) + " gates", 0.3f, 1.0f, 0.5f);
    }

    void run_shots() {
        shot_outcomes.clear();
        for (int i = 0; i < shot_count; ++i) {
            StateVector temp(n_qubits);
            circuit.run(temp, rng);
            int outcome = temp.measure_all(rng);
            shot_outcomes[outcome]++;
        }
        shots_ready = true;
        log("Ran " + std::to_string(shot_count) + " shots", 0.3f, 1.0f, 0.5f);
    }

    void load_algorithm(const std::string& name, const Circuit& c, int required_q) {
        if(n_qubits != required_q) {
            n_qubits = required_q;
        }
        circuit = c;
        run_circuit();
        log("Loaded algorithm: " + name, 0.9f, 0.7f, 0.2f);
    }
};