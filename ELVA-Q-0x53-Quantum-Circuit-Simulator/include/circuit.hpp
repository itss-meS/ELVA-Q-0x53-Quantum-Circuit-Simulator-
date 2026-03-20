#pragma once
#include "gates.hpp"
#include <vector>
#include <string>
#include <map>

enum class GateType { H, X, Y, Z, S, Sdg, T, Tdg, SX, Rx, Ry, Rz, P, U3, CNOT, CZ, SWAP, CU, Toffoli, ISWAP, Measure, MeasureAll };

struct GateOp {
    GateType type;
    std::vector<int> qubits;
    std::vector<double> params;
    std::string name;
};

struct RunResult {
    bool measured = false;
    int outcome = -1;
    std::map<int, int> partial_outcomes;
};

class Circuit {
public:
    std::vector<GateOp> ops;

    Circuit& h(int q) { ops.push_back({GateType::H, {q}, {}, "H"}); return *this; }
    Circuit& x(int q) { ops.push_back({GateType::X, {q}, {}, "X"}); return *this; }
    Circuit& y(int q) { ops.push_back({GateType::Y, {q}, {}, "Y"}); return *this; }
    Circuit& z(int q) { ops.push_back({GateType::Z, {q}, {}, "Z"}); return *this; }
    Circuit& s(int q) { ops.push_back({GateType::S, {q}, {}, "S"}); return *this; }
    Circuit& t(int q) { ops.push_back({GateType::T, {q}, {}, "T"}); return *this; }
    Circuit& rx(int q, double theta) { ops.push_back({GateType::Rx, {q}, {theta}, "Rx"}); return *this; }
    Circuit& ry(int q, double theta) { ops.push_back({GateType::Ry, {q}, {theta}, "Ry"}); return *this; }
    Circuit& rz(int q, double theta) { ops.push_back({GateType::Rz, {q}, {theta}, "Rz"}); return *this; }
    Circuit& cnot(int c, int t) { ops.push_back({GateType::CNOT, {c, t}, {}, "CX"}); return *this; }
    Circuit& cz(int c, int t) { ops.push_back({GateType::CZ, {c, t}, {}, "CZ"}); return *this; }
    Circuit& swap(int a, int b) { ops.push_back({GateType::SWAP, {a, b}, {}, "SWAP"}); return *this; }
    Circuit& toffoli(int c0, int c1, int t) { ops.push_back({GateType::Toffoli, {c0, c1, t}, {}, "CCX"}); return *this; }
    Circuit& measure(int q) { ops.push_back({GateType::Measure, {q}, {}, "M"}); return *this; }
    Circuit& measure_all() { ops.push_back({GateType::MeasureAll, {}, {}, "M_ALL"}); return *this; }

    RunResult run(StateVector& sv, std::mt19937& rng) {
        RunResult res;
        for (const auto& op : ops) {
            switch(op.type) {
                case GateType::H: GateApply::apply1(sv, gates::H(), op.qubits[0]); break;
                case GateType::X: GateApply::apply1(sv, gates::X(), op.qubits[0]); break;
                case GateType::Y: GateApply::apply1(sv, gates::Y(), op.qubits[0]); break;
                case GateType::Z: GateApply::apply1(sv, gates::Z(), op.qubits[0]); break;
                case GateType::S: GateApply::apply1(sv, gates::S(), op.qubits[0]); break;
                case GateType::T: GateApply::apply1(sv, gates::T(), op.qubits[0]); break;
                case GateType::Rx: GateApply::apply1(sv, gates::Rx(op.params[0]), op.qubits[0]); break;
                case GateType::Ry: GateApply::apply1(sv, gates::Ry(op.params[0]), op.qubits[0]); break;
                case GateType::Rz: GateApply::apply1(sv, gates::Rz(op.params[0]), op.qubits[0]); break;
                case GateType::CNOT: GateApply::cnot(sv, op.qubits[0], op.qubits[1]); break;
                case GateType::CZ: GateApply::cz(sv, op.qubits[0], op.qubits[1]); break;
                case GateType::SWAP: GateApply::swap(sv, op.qubits[0], op.qubits[1]); break;
                case GateType::Toffoli: GateApply::toffoli(sv, op.qubits[0], op.qubits[1], op.qubits[2]); break;
                case GateType::Measure:
                    res.partial_outcomes[op.qubits[0]] = sv.measure_qubit(op.qubits[0], rng);
                    break;
                case GateType::MeasureAll:
                    res.outcome = sv.measure_all(rng);
                    res.measured = true;
                    break;
                default: break;
            }
        }
        return res;
    }

    void clear() { ops.clear(); }
    void draw() const {} // ASCII visualization placeholder
};