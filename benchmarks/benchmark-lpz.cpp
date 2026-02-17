#include <iostream>
#include <fstream>
#include <benchmark/benchmark.h>
#include "lpz.h"
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

namespace {
    std::vector<uint8_t> g_input;
}

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

static void BM_ZSTD_Compress_Fast(benchmark::State& state) {
    int level = 1; // fast mode
    size_t last_size = 0;

    for (auto _ : state) {
        size_t bound = ZSTD_compressBound(g_input.size());
        std::vector<uint8_t> compressed(bound);

        size_t cSize = ZSTD_compress(compressed.data(), bound,
            g_input.data(), g_input.size(),
            level);
        if (ZSTD_isError(cSize)) {
            state.SkipWithError(ZSTD_getErrorName(cSize));
        }

        compressed.resize(cSize);
        last_size = cSize;
        benchmark::DoNotOptimize(compressed);
    }

    state.counters["Ratio"] = static_cast<double>(last_size) / g_input.size();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_ZSTD_Compress_High(benchmark::State& state) {
    int level = 19; // high compression
    size_t last_size = 0;

    for (auto _ : state) {
        size_t bound = ZSTD_compressBound(g_input.size());
        std::vector<uint8_t> compressed(bound);

        size_t cSize = ZSTD_compress(compressed.data(), bound,
            g_input.data(), g_input.size(),
            level);
        if (ZSTD_isError(cSize)) {
            state.SkipWithError(ZSTD_getErrorName(cSize));
        }

        compressed.resize(cSize);
        last_size = cSize;
        benchmark::DoNotOptimize(compressed);
    }

    state.counters["Ratio"] = static_cast<double>(last_size) / g_input.size();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_ZSTD_Decompress_Fast(benchmark::State& state) {
    int level = 1;
    size_t bound = ZSTD_compressBound(g_input.size());
    std::vector<uint8_t> compressed(bound);

    size_t cSize = ZSTD_compress(compressed.data(), bound,
        g_input.data(), g_input.size(),
        level);
    if (ZSTD_isError(cSize)) state.SkipWithError(ZSTD_getErrorName(cSize));
    compressed.resize(cSize);

    for (auto _ : state) {
        std::vector<uint8_t> decomp(g_input.size());
        size_t dSize = ZSTD_decompress(decomp.data(), decomp.size(),
            compressed.data(), compressed.size());
        if (ZSTD_isError(dSize)) state.SkipWithError(ZSTD_getErrorName(dSize));

        benchmark::DoNotOptimize(decomp);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_ZSTD_Decompress_High(benchmark::State& state) {
    int level = 19;
    size_t bound = ZSTD_compressBound(g_input.size());
    std::vector<uint8_t> compressed(bound);

    size_t cSize = ZSTD_compress(compressed.data(), bound,
        g_input.data(), g_input.size(),
        level);
    if (ZSTD_isError(cSize)) state.SkipWithError(ZSTD_getErrorName(cSize));
    compressed.resize(cSize);

    for (auto _ : state) {
        std::vector<uint8_t> decomp(g_input.size());
        size_t dSize = ZSTD_decompress(decomp.data(), decomp.size(),
            compressed.data(), compressed.size());
        if (ZSTD_isError(dSize)) state.SkipWithError(ZSTD_getErrorName(dSize));

        benchmark::DoNotOptimize(decomp);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

BENCHMARK(BM_ZSTD_Compress_Fast);
BENCHMARK(BM_ZSTD_Compress_High);
BENCHMARK(BM_ZSTD_Decompress_Fast);
BENCHMARK(BM_ZSTD_Decompress_High);


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: lpz-benchmark.exe <input_file>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cout << "Failed to open file\n";
        return 1;
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0);

    g_input.resize(size);
    file.read(reinterpret_cast<char*>(g_input.data()), size);

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}