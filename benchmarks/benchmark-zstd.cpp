#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>
#include <benchmark/benchmark.h>

extern std::vector<uint8_t> g_input;

static void BM_ZSTD_Compress_Fast(benchmark::State& state) {
    int level = 1; 
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
    int level = 19; 
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