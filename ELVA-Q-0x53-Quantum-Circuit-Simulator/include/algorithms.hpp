#pragma once
#include "circuit.hpp"

namespace quantum::algorithms {
    inline Circuit bell_phi_plus() { Circuit c; return c.h(0).cnot(0, 1); }
    inline Circuit bell_phi_minus() { Circuit c; return c.x(1).h(0).cnot(0, 1); }
    inline Circuit bell_psi_plus() { Circuit c; return c.x(0).h(0).cnot(0, 1); }
    inline Circuit bell_psi_minus() { Circuit c; return c.x(0).x(1).h(0).cnot(0, 1); }

    inline Circuit ghz(int n) {
        Circuit c; c.h(0);
        for(int i=1; i<n; ++i) c.cnot(0, i);
        return c;
    }

    inline Circuit qft(int n) {
        Circuit c;
        for (int i = 0; i < n; ++i) {
            c.h(i);
            for (int j = i + 1; j < n; ++j) {
                double theta = M_PI / (1 << (j - i));
                c.ops.push_back({GateType::CU, {j, i}, {theta, 0, 0}, "CU1"}); // simplified param passing logic for demo
            }
        }
        for (int i = 0; i < n / 2; ++i) c.swap(i, n - 1 - i);
        return c;
    }

    inline Circuit grover_2q() {
        Circuit c;
        c.h(0).h(1);
        c.cz(0, 1);
        c.h(0).h(1).z(0).z(1).cz(0, 1).h(0).h(1);
        return c;
    }

    inline Circuit teleportation() {
        Circuit c;
        c.h(1).cnot(1, 2);
        c.cnot(0, 1).h(0);
        c.cnot(1, 2).cz(0, 2);
        return c;
    }

    inline Circuit bernstein_vazirani() {
        Circuit c; // Secret 101 on 3 qubits + 1 ancilla
        c.x(3).h(0).h(1).h(2).h(3);
        c.cnot(0, 3).cnot(2, 3);
        c.h(0).h(1).h(2);
        return c;
    }

    inline Circuit deutsch_jozsa_constant(int n) {
        Circuit c;
        for(int i=0; i<n-1; ++i) c.h(i);
        c.x(n-1).h(n-1);
        for(int i=0; i<n-1; ++i) c.h(i);
        return c;
    }

    inline Circuit qaoa_ring(int n, double gamma, double beta) {
        Circuit c;
        for(int i=0; i<n; ++i) c.h(i);
        for(int i=0; i<n; ++i) { c.cnot(i, (i+1)%n).rz((i+1)%n, gamma).cnot(i, (i+1)%n); }
        for(int i=0; i<n; ++i) c.rx(i, beta);
        return c;
    }

    inline Circuit superdense_encode(int b1, int b0) {
        Circuit c;
        c.h(0).cnot(0, 1);
        if(b0) c.x(0);
        if(b1) c.z(0);
        c.cnot(0, 1).h(0);
        return c;
    }

    inline Circuit qrng(int n) {
        Circuit c;
        for(int i=0; i<n; ++i) c.h(i);
        return c;
    }

    inline Circuit phase_estimation_demo() {
        Circuit c;
        c.x(3).h(0).h(1).h(2);
        c.ops.push_back({GateType::CU, {2, 3}, {M_PI/4}, "CU"});
        return c;
    }
}