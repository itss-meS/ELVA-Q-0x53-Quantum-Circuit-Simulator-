#pragma once
#include "complex.hpp"
#include "statevector.hpp"
#include <algorithm>

struct Matrix2 {
    Complex u00, u01, u10, u11;
    Matrix2 operator*(const Matrix2& o) const {
        return {
            u00*o.u00 + u01*o.u10, u00*o.u01 + u01*o.u11,
            u10*o.u00 + u11*o.u10, u10*o.u01 + u11*o.u11
        };
    }
    Matrix2 dagger() const { return {u00.conj(), u10.conj(), u01.conj(), u11.conj()}; }
};

namespace gates {
    inline Matrix2 I() { return {Complex::ONE, Complex::ZERO, Complex::ZERO, Complex::ONE}; }
    inline Matrix2 X() { return {Complex::ZERO, Complex::ONE, Complex::ONE, Complex::ZERO}; }
    inline Matrix2 Y() { return {Complex::ZERO, {0,-1}, {0,1}, Complex::ZERO}; }
    inline Matrix2 Z() { return {Complex::ONE, Complex::ZERO, Complex::ZERO, {-1,0}}; }
    inline Matrix2 H() { return {Complex::INV_SQRT2, Complex::INV_SQRT2, Complex::INV_SQRT2, {-Complex::INV_SQRT2.real, 0}}; }
    inline Matrix2 S() { return {Complex::ONE, Complex::ZERO, Complex::ZERO, Complex::I}; }
    inline Matrix2 Sdg() { return S().dagger(); }
    inline Matrix2 T() { return {Complex::ONE, Complex::ZERO, Complex::ZERO, Complex::polar(1.0, M_PI/4.0)}; }
    inline Matrix2 Tdg() { return T().dagger(); }
    inline Matrix2 SX() { return { {0.5, 0.5}, {0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5} }; }

    inline Matrix2 Rx(double theta) { return { {std::cos(theta/2), 0}, {0, -std::sin(theta/2)}, {0, -std::sin(theta/2)}, {std::cos(theta/2), 0} }; }
    inline Matrix2 Ry(double theta) { return { {std::cos(theta/2), 0}, {-std::sin(theta/2), 0}, {std::sin(theta/2), 0}, {std::cos(theta/2), 0} }; }
    inline Matrix2 Rz(double theta) { return { Complex::polar(1.0, -theta/2), Complex::ZERO, Complex::ZERO, Complex::polar(1.0, theta/2) }; }
    inline Matrix2 P(double theta) { return { Complex::ONE, Complex::ZERO, Complex::ZERO, Complex::polar(1.0, theta) }; }
    inline Matrix2 U3(double theta, double phi, double lam) {
        return {
            {std::cos(theta/2), 0},
            Complex::polar(-std::sin(theta/2), lam),
            Complex::polar(std::sin(theta/2), phi),
            Complex::polar(std::cos(theta/2), phi + lam)
        };
    }
}

class GateApply {
public:
    static void apply1(StateVector& sv, const Matrix2& m, int q) {
        for (size_t i = 0; i < sv.size; ++i) {
            if (((i >> q) & 1) == 0) {
                size_t j = i | (1ull << q);
                Complex v0 = sv.amplitudes[i];
                Complex v1 = sv.amplitudes[j];
                sv.amplitudes[i] = m.u00 * v0 + m.u01 * v1;
                sv.amplitudes[j] = m.u10 * v0 + m.u11 * v1;
            }
        }
    }
    static void cnot(StateVector& sv, int c, int t) {
        for (size_t i = 0; i < sv.size; ++i) {
            if (((i >> c) & 1) && (((i >> t) & 1) == 0)) {
                std::swap(sv.amplitudes[i], sv.amplitudes[i | (1ull << t)]);
            }
        }
    }
    static void cz(StateVector& sv, int c, int t) {
        for (size_t i = 0; i < sv.size; ++i) {
            if (((i >> c) & 1) && ((i >> t) & 1)) {
                sv.amplitudes[i] = sv.amplitudes[i] * -1.0;
            }
        }
    }
    static void swap(StateVector& sv, int a, int b) {
        for (size_t i = 0; i < sv.size; ++i) {
            int bit_a = (i >> a) & 1;
            int bit_b = (i >> b) & 1;
            if (bit_a == 0 && bit_b == 1) {
                std::swap(sv.amplitudes[i], sv.amplitudes[i ^ (1ull << a) ^ (1ull << b)]);
            }
        }
    }
    static void cu(StateVector& sv, const Matrix2& m, int c, int t) {
        for (size_t i = 0; i < sv.size; ++i) {
            if (((i >> c) & 1) && (((i >> t) & 1) == 0)) {
                size_t j = i | (1ull << t);
                Complex v0 = sv.amplitudes[i];
                Complex v1 = sv.amplitudes[j];
                sv.amplitudes[i] = m.u00 * v0 + m.u01 * v1;
                sv.amplitudes[j] = m.u10 * v0 + m.u11 * v1;
            }
        }
    }
    static void toffoli(StateVector& sv, int c0, int c1, int t) {
        for (size_t i = 0; i < sv.size; ++i) {
            if (((i >> c0) & 1) && ((i >> c1) & 1) && (((i >> t) & 1) == 0)) {
                std::swap(sv.amplitudes[i], sv.amplitudes[i | (1ull << t)]);
            }
        }
    }
    static void iswap(StateVector& sv, int a, int b) {
        for (size_t i = 0; i < sv.size; ++i) {
            int bit_a = (i >> a) & 1;
            int bit_b = (i >> b) & 1;
            if (bit_a == 0 && bit_b == 1) {
                size_t j = i ^ (1ull << a) ^ (1ull << b);
                Complex tmp = sv.amplitudes[i];
                sv.amplitudes[i] = sv.amplitudes[j] * Complex(0, 1);
                sv.amplitudes[j] = tmp * Complex(0, 1);
            }
        }
    }
};