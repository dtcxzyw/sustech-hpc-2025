#include <complex>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <omp.h>
#include <utility>
#include <vector>

struct Number {
  using NumberType = int32_t;
  NumberType Val;

  constexpr static NumberType Zero = 0;
  constexpr static NumberType PosOne = 1;
  constexpr static NumberType NegOne = -1;
  constexpr static NumberType PosInvSqrt2 = 2;
  constexpr static NumberType NegInvSqrt2 = -2;
  constexpr static NumberType PosInv2 = 4;
  constexpr static NumberType NegInv2 = -4;

  constexpr Number(NumberType V) : Val(V) {}
  Number operator-() const { return {-Val}; }
  double square() const {
    switch (Val) {
    case Zero:
      return 0.0;
    case PosOne:
    case NegOne:
      return 1.0;
    case PosInv2:
    case NegInv2:
      return 0.25;
    case PosInvSqrt2:
    case NegInvSqrt2:
      return 0.5;
    default:
      __builtin_unreachable(); // Invalid value
    }
  }

  // Compute (A + B) / sqrt(2)
  Number addDivSqrt2(Number RHS) const {
    NumberType Sum = Val + RHS.Val;
    if (Val == Zero || RHS.Val == Zero)
      return Sum << 1;
    return Sum >> 2;
  }

  double materialize() const {
    switch (Val) {
    case Zero:
      return 0.0;
    case PosOne:
      return 1.0;
    case NegOne:
      return -1.0;
    case PosInv2:
      return 0.5;
    case NegInv2:
      return -0.5;
    case PosInvSqrt2:
      return 0.70710678118654752440084436210485; // 1/sqrt(2)
    case NegInvSqrt2:
      return -0.70710678118654752440084436210485; // -1/sqrt(2)
    default:
      __builtin_unreachable(); // Invalid value
    }
  }
};

struct Complex {
  Number Real, Imag;
  constexpr Complex(Number R) : Real(R), Imag(Number::Zero) {}
  constexpr Complex(Number R, Number I) : Real(R), Imag(I) {}
  static constexpr Complex Zero() { return {Number::Zero}; }
  static constexpr Complex One() { return {Number::PosOne}; }
  Complex operator-() const { return {-Real, -Imag}; }
  Complex multiI() const { return {-Imag, Real}; }
  Complex addDivSqrt2(Complex RHS) const {
    return {Real.addDivSqrt2(RHS.Real), Imag.addDivSqrt2(RHS.Imag)};
  }
  std::complex<double> materialize() const {
    return {Real.materialize(), Imag.materialize()};
  }
};

struct Qubit {
  Complex Alpha, Beta;

  void applyH() {
    // Alpha = (Alpha + Beta) / sqrt(2)
    // Beta = (Alpha - Beta) / sqrt(2)
    auto NewAlpha = Alpha.addDivSqrt2(Beta);
    auto NewBeta = Alpha.addDivSqrt2(-Beta);
    Alpha = NewAlpha;
    Beta = NewBeta;
  }
  void applyX() { std::swap(Alpha, Beta); }
  void applyY() {
    std::swap(Alpha, Beta);
    Alpha = -Alpha.multiI();
    Beta = Beta.multiI();
  }
  void applyZ() { Beta = -Beta; }
  void applyS() { Beta = Beta.multiI(); }
};

struct Gate {
  Qubit C1, C2;

  Gate()
      : C1{Qubit{Complex::One(), Complex::Zero()}},
        C2{Qubit{Complex::Zero(), Complex::One()}} {}
  void applyH() {
    C1.applyH();
    C2.applyH();
  }
  void applyX() {
    C1.applyX();
    C2.applyX();
  }
  void applyY() {
    C1.applyY();
    C2.applyY();
  }
  void applyZ() {
    C1.applyZ();
    C2.applyZ();
  }
  void applyS() {
    C1.applyS();
    C2.applyS();
  }

  void apply(std::complex<double> &Alpha, std::complex<double> &Beta) const {
    auto A00 = C1.Alpha.materialize();
    auto A01 = C2.Alpha.materialize();
    auto A10 = C1.Beta.materialize();
    auto A11 = C2.Beta.materialize();

    auto NewAlpha = A00 * Alpha + A01 * Beta;
    auto NewBeta = A10 * Alpha + A11 * Beta;
    Alpha = NewAlpha;
    Beta = NewBeta;
  }
};

void simulate(size_t N, const char *Gates, std::complex<double> &Alpha,
              std::complex<double> &Beta) {
  int NumThreads = omp_get_max_threads();
  std::cerr << "Using " << NumThreads << " threads.\n";
  size_t ChunkSize = N / NumThreads;

  std::vector<Gate> GatesVec(NumThreads);

#pragma omp parallel for
  for (int I = 0; I < NumThreads; ++I) {
    Gate G;
    size_t Start = I * ChunkSize;
    size_t End = Start + ChunkSize;
    for (size_t J = Start; J < End; ++J) {
      switch (Gates[J]) {
      case 'H':
        G.applyH();
        break;
      case 'X':
        G.applyX();
        break;
      case 'Y':
        G.applyY();
        break;
      case 'Z':
        G.applyZ();
        break;
      case 'S':
        G.applyS();
        break;
      default:
        __builtin_unreachable(); // Invalid gate
      }
    }

    GatesVec[I] = G;
  }

  Alpha = 1.0;
  Beta = 0.0;
  for (auto &G : GatesVec)
    G.apply(Alpha, Beta);
}
