#include <complex>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <omp.h>
#include <utility>
#include <vector>

struct Number {
  int32_t Val;

  constexpr static int32_t Zero = 0;
  constexpr static int32_t PosOne = 1;
  constexpr static int32_t NegOne = -1;
  constexpr static int32_t PosInv2 = 2;
  constexpr static int32_t NegInv2 = -2;
  constexpr static int32_t PosInvSqrt2 = 3;
  constexpr static int32_t NegInvSqrt2 = -3;

  constexpr Number(int32_t V) : Val(V) {}
  Number operator-() const { return {-Val}; }

  // Compute A / sqrt(2)
  Number divSqrt2() const {
    switch (Val) {
    case PosOne:
      return {PosInvSqrt2};
    case NegOne:
      return {NegInvSqrt2};
    case PosInvSqrt2:
      return {PosInv2};
    case NegInvSqrt2:
      return {NegInv2};
    default:
      __builtin_unreachable(); // Invalid case
    }
  }
  // Compute (A + B) / sqrt(2)
  Number addDivSqrt2(Number RHS) const {
    if (Val + RHS.Val == 0)
      return {Zero};
    if (Val == Zero)
      return RHS.divSqrt2();
    if (RHS.Val == Zero)
      return divSqrt2();

    if (Val != RHS.Val)
      __builtin_unreachable();

    switch (Val) {
    case PosInv2:
      return {PosInvSqrt2};
    case NegInv2:
      return {NegInvSqrt2};
    case PosInvSqrt2:
      return {PosOne};
    case NegInvSqrt2:
      return {NegOne};
    default:
      __builtin_unreachable(); // Invalid case
    }
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
