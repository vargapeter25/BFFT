#ifndef COMPLEX__H
#define COMPLEX__H

#include <cmath>
#include <fstream>
#include "mpl.hpp"

struct Complex{
    double real;
    double imag;

    Complex() : real(0), imag(0) {}
    Complex(double val_r) : real(val_r), imag(0) {}
    Complex(double val_r, double val_i) : real(val_r), imag(val_i) {}

    bool operator==(const Complex& z) const { return real == z.real && imag == z.imag; }

    Complex operator-() const { return Complex(-real, -imag); }

    Complex& operator+=(double x) { real += x; return (*this); }
    Complex& operator-=(double x) { real -= x; return (*this); }
    Complex& operator*=(double x) { real *= x; imag *= x; return (*this); }
    Complex& operator/=(double x) { 
        real /= x;
        imag /= x; 
        return (*this); 
    }

    Complex& operator+=(const Complex& z) { real += z.real; imag += z.imag; return (*this); }
    Complex& operator-=(const Complex& z) { real -= z.real; imag -= z.imag; return (*this); }
    Complex& operator*=(const Complex& z) { double tmp = real * z.real - imag * z.imag; imag = real * z.imag + imag * z.real; real = tmp; return (*this); }
    Complex& operator/=(const Complex& z) { this->conj_mult(z); (*this) /= norm(z); return (*this); }

    Complex operator+(double x) const { Complex result(*this); return result += x; }
    Complex operator-(double x) const { Complex result(*this); return result -= x; }
    Complex operator*(double x) const { Complex result(*this); return result *= x; }
    Complex operator/(double x) const { Complex result(*this); return result /= x; }

    Complex operator+(const Complex& z) const { Complex result(*this); return result += z; }
    Complex operator-(const Complex& z) const { Complex result(*this); return result -= z; }
    Complex operator*(const Complex& z) const { Complex result(*this); return result *= z; }
    Complex operator/(const Complex& z) const { Complex result(*this); return result /= z; }

    Complex& conj() { imag = -imag; return (*this); }
    Complex& conj_mult(const Complex& z) { double tmp = real * z.real + imag * z.imag; imag = - real * z.imag + imag * z.real; real = tmp; return (*this); }

    static inline double norm(const Complex& z) { return z.real * z.real + z.imag * z.imag; }
    static inline double abs(const Complex& z) { return std::sqrt(norm(z)); }
    static inline double angle(const Complex& z) { return std::atan2(z.imag, z.real); }
    static inline Complex sqrt(const Complex& z);
    static inline Complex conj(const Complex& z) { return Complex(z.real, -z.imag); }
    static inline Complex conj_mult(const Complex& z1, const Complex& z2) { Complex result(z1); return result.conj_mult(z2); }
    static inline Complex polar(double radius, double angle) { return Complex(std::cos(angle), std::sin(angle)) * radius; }
};

inline std::ostream& operator<<(std::ostream& os, const Complex& z) { os << '(' << z.real << ",  " << z.imag << ')'; return os; }

inline Complex Complex::sqrt(const Complex& z) { 
    double r = abs(z); 
    if(r == 0.0){
        return Complex(0);
    }

    Complex result = z / r;

    double a = std::sqrt(std::abs((result.real + 1.0) * 0.5));
    double b = std::sqrt(std::abs((1.0 - result.real) * 0.5));

    if(result.imag < 0) b = -b;

    return Complex(a, b) * std::sqrt(r);
}

#endif //COMPLEX__H