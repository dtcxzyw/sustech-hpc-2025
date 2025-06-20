#include <complex>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <set>
#include <utility>

// struct Number {
//   enum NumberKind {
//     Zero,
//     PosOne,
//     NegOne,
//     PosInvSqrt2,
//     NegInvSqrt2,
//   } Val;

//   void negate() {
//     // TODO
//   }
// };

// struct Complex {
//   Number Real, Imag;

//   static constexpr Complex Zero() { return {Number::Zero, Number::Zero}; }

//   static constexpr Complex One() { return {Number::PosOne, Number::Zero}; }

//   static constexpr Complex I() { return {Number::Zero, Number::PosOne}; }

//   void negate() {
//     Real.negate();
//     Imag.negate();
//   }

//   void multiI() {
//     std::swap(Real, Imag);
//     Real.negate();
//   }

//   double measure() const { return 0.0; }
// };

// struct Qubit {
//   Complex Alpha, Beta;

//   void applyH() {}
//   void applyX() { std::swap(Alpha, Beta); }
//   void applyY() {
//     std::swap(Alpha, Beta);
//     Alpha.multiI();
//     Alpha.negate();
//     Beta.multiI();
//   }
//   void applyZ() { Beta.negate(); }
// };

using namespace std::complex_literals;

constexpr double InvSqrt2 = 0.70710678118654752440084436210485; // 1/sqrt(2)

struct Qubit {
  std::complex<double> Alpha, Beta;

  void applyH() {
    auto NewAlpha = (Alpha + Beta) * InvSqrt2;
    auto NewBeta = (Alpha - Beta) * InvSqrt2;
    Alpha = NewAlpha;
    Beta = NewBeta;
  }
  void applyX() { std::swap(Alpha, Beta); }
  void applyY() {
    auto NewAlpha = Beta * -1i;
    auto NewBeta = Alpha * 1i;
    Alpha = NewAlpha;
    Beta = NewBeta;
  }
  void applyZ() { Beta = -Beta; }
  void applyS() { Beta *= 1i; }
};

std::complex<double> round(std::complex<double> X) {
  return {std::round(X.real() * 100), std::round(X.imag() * 100)};
}

struct Less {
  bool operator()(const std::complex<double> &LHS,
                  const std::complex<double> &RHS) const {
    if (LHS.real() != RHS.real())
      return LHS.real() < RHS.real();
    return LHS.imag() < RHS.imag();
  }
};

void simulate(size_t N, const char *Gates, std::complex<double> &Alpha,
              std::complex<double> &Beta) {
  Qubit State{1.0, 0.0};
  // std::set<std::complex<double>, Less> UniqueStates;
  for (size_t i = 0; i < N; ++i) {
    switch (Gates[i]) {
    case 'H':
      State.applyH();
      break;
    case 'X':
      State.applyX();
      break;
    case 'Y':
      State.applyY();
      break;
    case 'Z':
      State.applyZ();
      break;
    case 'S':
      State.applyS();
      break;
    default:
      __builtin_unreachable(); // Invalid gate
    }

    // std::cerr << "After gate " << Gates[i] << ": "
    //           << "Alpha = " << State.Alpha << ", "
    //           << "Beta = " << State.Beta << "\n";
    // UniqueStates.insert(round(State.Alpha));
    // UniqueStates.insert(round(State.Beta));
  }

  // std::cerr << "Unique states count: " << UniqueStates.size() << "\n";
  // for (const auto &State : UniqueStates) {
  //   std::cerr << State << '\n';
  // }

  Alpha = State.Alpha;
  Beta = State.Beta;
}
