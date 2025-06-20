#include <atomic>
#include <chrono>
#include <complex>
#include <cstdio>
#include <random>
#include <vector>

void simulate(size_t N, const char *Gates, std::complex<double> &Alpha,
              std::complex<double> &Beta);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <length> <seed>\n", argv[0]);
    return 1;
  }

  int N = std::atoi(argv[1]);
  int Seed = std::atoi(argv[2]);

  std::mt19937 Rng(Seed);
  std::uniform_int_distribution<int> Dist(0, 4);
  std::vector<char> Gates(N);
  for (int i = 0; i < N; ++i) {
    int gate = Dist(Rng);
    Gates[i] = "HXYZS"[gate];
  }

  std::complex<double> Alpha = {}, Beta = {};

  std::atomic_signal_fence(std::memory_order_acq_rel);
  auto start = std::chrono::high_resolution_clock::now();
  std::atomic_signal_fence(std::memory_order_acq_rel);
  simulate(N, Gates.data(), Alpha, Beta);
  std::atomic_signal_fence(std::memory_order_acq_rel);
  auto end = std::chrono::high_resolution_clock::now();
  std::atomic_signal_fence(std::memory_order_acq_rel);

  std::chrono::duration<double, std::milli> duration = end - start;
  printf("Final state: alpha = %.12f + %.12fi, beta = %.12f + %.12fi\n",
         Alpha.real(), Alpha.imag(), Beta.real(), Beta.imag());
  printf("Time taken: %.2f ms\n", duration.count());

  return 0;
}
