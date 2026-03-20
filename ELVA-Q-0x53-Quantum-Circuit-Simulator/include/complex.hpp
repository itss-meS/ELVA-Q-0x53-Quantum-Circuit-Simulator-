#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

struct Complex {
    double real = 0.0;
    double imag = 0.0;

    Complex() = default;
    Complex(double r, double i) : real(r), imag(i) {}

    Complex operator+(const Complex& o) const { return {real + o.real, imag + o.imag}; }
    Complex operator-(const Complex& o) const { return {real - o.real, imag - o.imag}; }
    Complex operator*(const Complex& o) const { return {real * o.real - imag * o.imag, real * o.imag + imag * o.real}; }
    Complex operator*(double s) const { return {real * s, imag * s}; }
    Complex operator/(double s) const { return {real / s, imag / s}; }

    Complex conj() const { return {real, -imag}; }
    double norm2() const { return real * real + imag * imag; }
    double norm() const { return std::sqrt(norm2()); }
    double phase() const { return std::atan2(imag, real); }

    static Complex polar(double r, double theta) { return {r * std::cos(theta), r * std::sin(theta)}; }

    static const Complex ZERO;
    static const Complex ONE;
    static const Complex I;
    static const Complex INV_SQRT2;
};

inline const Complex Complex::ZERO = {0.0, 0.0};
inline const Complex Complex::ONE = {1.0, 0.0};
inline const Complex Complex::I = {0.0, 1.0};
inline const Complex Complex::INV_SQRT2 = {1.0 / std::sqrt(2.0), 0.0};