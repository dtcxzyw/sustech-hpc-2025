#include <chrono>
#include <complex>
#include <cstdio>
#include <vector>

void simulate(size_t N, const char *Gates, std::complex<double> &Alpha,
              std::complex<double> &Beta);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
    return 1;
  }

  const char *output_file = argv[1];
  FILE *file = fopen(output_file, "rb");
  if (!file) {
    perror("Failed to open file");
    return 1;
  }

  int N;
  [[maybe_unused]] auto Res1 = fread(&N, sizeof(int), 1, file);
  std::vector<char> Gates(N);
  [[maybe_unused]] auto Res2 = fread(Gates.data(), sizeof(char), N, file);
  fclose(file);

  std::complex<double> Alpha = {}, Beta = {};

  auto start = std::chrono::high_resolution_clock::now();
  simulate(N, Gates.data(), Alpha, Beta);
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;
  printf("Final state: alpha = %.12f + %.12fi, beta = %.12f + %.12fi\n",
         Alpha.real(), Alpha.imag(), Beta.real(), Beta.imag());
  printf("Time taken: %.2f ms\n", duration.count());

  return 0;
}
