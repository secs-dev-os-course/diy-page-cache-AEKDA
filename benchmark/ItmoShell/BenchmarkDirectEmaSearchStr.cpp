#include <benchmark/benchmark.h>

#include "Funcs.hpp"

void BenchmarkDirectEmaSearchStr(benchmark::State &state) {
  const std::string filepath = "test/ema-search-str.txt";
  const std::string substring = "ikhchvcuohuleuguvpnglefzspyvpqwkclnkewtt";
  // const size_t repeat = 82;
  size_t block_size = state.range(0);

  for (auto _ : state) {
    auto found = monolith::DirectEmaSearchStr(filepath, substring, block_size);
    benchmark::DoNotOptimize(found);
    found = monolith::DirectEmaSearchStr(filepath, substring, block_size);
    benchmark::DoNotOptimize(found);
  }
}

#define BLOCK_SIZE 4096

BENCHMARK(BenchmarkDirectEmaSearchStr)
    ->Arg(BLOCK_SIZE)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();