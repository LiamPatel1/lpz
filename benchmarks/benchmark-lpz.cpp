#include <benchmark/benchmark.h>
#include "lpz.h"

extern std::vector<uint8_t> g_input;

static void BM_LPZ_Compress(benchmark::State& state) {

    size_t last_size = 0;

    auto test = lpz::compress(g_input);
    if (!test) state.SkipWithError(test.error().m);

    for (auto _ : state) {
        auto result = lpz::compress(g_input).value();
        last_size = result.size();
        benchmark::DoNotOptimize(result);
    }

    double ratio =
        static_cast<double>(last_size) / g_input.size();

    state.counters["Ratio"] = ratio;

    state.SetBytesProcessed(
        int64_t(state.iterations()) * int64_t(g_input.size())
    );
}
BENCHMARK(BM_LPZ_Compress);


static void BM_LPZ_Decompress(benchmark::State& state) {

    auto comp = lpz::compress(g_input);
    if (!comp) state.SkipWithError(comp.error().m);

    auto test = lpz::decompress(*comp);
    if (!test) state.SkipWithError(test.error().m);


    for (auto _ : state) {
        auto decomp = lpz::decompress(*comp).value();
        benchmark::DoNotOptimize(decomp);
    }

    state.SetBytesProcessed(
        int64_t(state.iterations()) * int64_t((*comp).size())
    );
}
BENCHMARK(BM_LPZ_Decompress);

