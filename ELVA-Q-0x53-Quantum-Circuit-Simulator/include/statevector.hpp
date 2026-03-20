#pragma once
#include "complex.hpp"
#include <vector>
#include <random>
#include <string>
#include <stdexcept>
#include <bitset>

class StateVector {
public:
    int n_qubits;
    size_t size;
    std::vector<Complex> amplitudes;

    StateVector(int n = 1) {
        if (n < 1 || n > 20) n = 1;
        n_qubits = n;
        size = 1ull << n;
        amplitudes.assign(size, Complex::ZERO);
        amplitudes[0] = Complex::ONE;
    }

    double norm() const {
        double n = 0;
        for (const auto& a : amplitudes) n += a.norm2();
        return std::sqrt(n);
    }

    void normalize() {
        double n = norm();
        if (n > 0) {
            for (auto& a : amplitudes) {
                a.real /= n;
                a.imag /= n;
            }
        }
    }

    double prob(size_t i) const { return amplitudes[i].norm2(); }

    double prob_qubit_one(int q) const {
        double p = 0;
        for (size_t i = 0; i < size; ++i) {
            if ((i >> q) & 1) p += prob(i);
        }
        return p;
    }

    size_t measure_all(std::mt19937& rng) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double r = dist(rng);
        double acc = 0.0;
        for (size_t i = 0; i < size; ++i) {
            acc += prob(i);
            if (r <= acc) {
                amplitudes.assign(size, Complex::ZERO);
                amplitudes[i] = Complex::ONE;
                return i;
            }
        }
        amplitudes.assign(size, Complex::ZERO);
        amplitudes[size - 1] = Complex::ONE;
        return size - 1;
    }

    int measure_qubit(int q, std::mt19937& rng) {
        double p1 = prob_qubit_one(q);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        int outcome = (dist(rng) < p1) ? 1 : 0;
        double norm_factor = outcome ? std::sqrt(p1) : std::sqrt(1.0 - p1);

        for (size_t i = 0; i < size; ++i) {
            if (((i >> q) & 1) == outcome) {
                amplitudes[i] = amplitudes[i] / norm_factor;
            } else {
                amplitudes[i] = Complex::ZERO;
            }
        }
        return outcome;
    }

    double expect_Z(int q) const {
        double exp = 0;
        for (size_t i = 0; i < size; ++i) {
            double p = prob(i);
            exp += ((i >> q) & 1) ? -p : p;
        }
        return exp;
    }

    double expect_X(int q) const {
        double exp = 0;
        for (size_t i = 0; i < size; ++i) {
            if (((i >> q) & 1) == 0) {
                size_t j = i | (1ull << q);
                exp += 2.0 * (amplitudes[i].real * amplitudes[j].real + amplitudes[i].imag * amplitudes[j].imag);
            }
        }
        return exp;
    }

    double expect_Y(int q) const {
        double exp = 0;
        for (size_t i = 0; i < size; ++i) {
            if (((i >> q) & 1) == 0) {
                size_t j = i | (1ull << q);
                exp += 2.0 * (amplitudes[i].real * amplitudes[j].imag - amplitudes[i].imag * amplitudes[j].real);
            }
        }
        return exp;
    }

    std::string ket_label(size_t i) const {
        std::string s = "|";
        for (int q = n_qubits - 1; q >= 0; --q) {
            s += ((i >> q) & 1) ? "1" : "0";
        }
        return s + "⟩";
    }

    std::vector<double> probabilities() const {
        std::vector<double> p(size);
        for (size_t i = 0; i < size; ++i) p[i] = prob(i);
        return p;
    }

    std::string to_string() const {
        return "StateVector(" + std::to_string(n_qubits) + "q)";
    }
};