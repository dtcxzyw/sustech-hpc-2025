#include <complex>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <utility>

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

  // // Compute (A + B) / sqrt(2)
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
  std::complex<double> materialize() const {
    return {Real.materialize(), Imag.materialize()};
  }

  bool operator!=(const Complex &Other) const {
    return Real.Val != Other.Real.Val || Imag.Val != Other.Imag.Val;
  }
  bool operator<(const Complex &Other) const {
    if (Real.Val != Other.Real.Val)
      return Real.Val < Other.Real.Val;
    return Imag.Val < Other.Imag.Val;
  }
};

struct Qubit {
  Complex Alpha, Beta;

  void applyH() {
    // Alpha = (Alpha + Beta) / sqrt(2)
    // Beta = (Alpha - Beta) / sqrt(2)
    auto NewAlphaReal = Alpha.Real.addDivSqrt2(Beta.Real);
    auto NewAlphaImag = Alpha.Imag.addDivSqrt2(Beta.Imag);
    auto NewBetaReal = Alpha.Real.addDivSqrt2(-Beta.Real);
    auto NewBetaImag = Alpha.Imag.addDivSqrt2(-Beta.Imag);
    Alpha = {NewAlphaReal, NewAlphaImag};
    Beta = {NewBetaReal, NewBetaImag};
  }
  void applyX() { std::swap(Alpha, Beta); }
  void applyY() {
    std::swap(Alpha, Beta);
    Alpha = -Alpha.multiI();
    Beta = Beta.multiI();
  }
  void applyZ() { Beta = -Beta; }
  void applyS() { Beta = Beta.multiI(); }

  bool operator<(const Qubit &RHS) const {
    if (Alpha != RHS.Alpha)
      return Alpha < RHS.Alpha;
    return Beta < RHS.Beta;
  }
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
  Gate G;
  std::set<Qubit> Qubits;
  for (size_t J = 0; J < N; ++J) {
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
    Qubits.insert(G.C1);
    Qubits.insert(G.C2);
  }

  std::cout << "Qubits: " << Qubits.size() << "\n";
  std::map<Qubit, uint32_t> QubitMap;
  uint32_t Idx = 0;
  for (const auto &Q : Qubits) {
    QubitMap[Q] = Idx;
    std::cout << Idx << ' ' << Q.Alpha.Real.Val << ' ' << Q.Alpha.Imag.Val
              << ' ' << Q.Beta.Real.Val << ' ' << Q.Beta.Imag.Val << '\n';
    ++Idx;
  }

  std::cout << '\n';
  for (const auto &Q : Qubits) {
    // std::cout << "Alpha: " << Q.Alpha.materialize()
    //           << ", Beta: " << Q.Beta.materialize() << "\n";
    auto Q1 = Q;
    Q1.applyH();
    auto Q2 = Q;
    Q2.applyX();
    auto Q3 = Q;
    Q3.applyY();
    auto Q4 = Q;
    Q4.applyZ();
    auto Q5 = Q;
    Q5.applyS();
    std::cout << QubitMap.at(Q) << ' ' << QubitMap.at(Q1) << ' '
              << QubitMap.at(Q2) << ' ' << QubitMap.at(Q3) << ' '
              << QubitMap.at(Q4) << ' ' << QubitMap.at(Q5) << '\n';
  }
}
