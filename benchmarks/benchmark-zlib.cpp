#include <zlib.h>
#include <benchmark/benchmark.h>

extern std::vector<uint8_t> g_input; 

static void BM_ZLIB_Compress_Fast(benchmark::State& state) {
    int level = Z_BEST_SPEED; 
    size_t last_size = 0;

    for (auto _ : state) {
        uLongf bound = compressBound(static_cast<uLong>(g_input.size()));
        std::vector<uint8_t> compressed(bound);

        uLongf out_len = bound;
        int ret = compress2(compressed.data(), &out_len,
            reinterpret_cast<const Bytef*>(g_input.data()),
            static_cast<uLong>(g_input.size()),
            level);
        if (ret != Z_OK) state.SkipWithError("zlib compress2 failed");

        compressed.resize(out_len);
        last_size = out_len;
        benchmark::DoNotOptimize(compressed);
    }

    state.counters["Ratio"] = static_cast<double>(last_size) / g_input.size();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_ZLIB_Compress_High(benchmark::State& state) {
    int level = Z_BEST_COMPRESSION; 
    size_t last_size = 0;

    for (auto _ : state) {
        uLongf bound = compressBound(static_cast<uLong>(g_input.size()));
        std::vector<uint8_t> compressed(bound);

        uLongf out_len = bound;
        int ret = compress2(compressed.data(), &out_len,
            reinterpret_cast<const Bytef*>(g_input.data()),
            static_cast<uLong>(g_input.size()),
            level);
        if (ret != Z_OK) state.SkipWithError("zlib compress2 failed");

        compressed.resize(out_len);
        last_size = out_len;
        benchmark::DoNotOptimize(compressed);
    }

    state.counters["Ratio"] = static_cast<double>(last_size) / g_input.size();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_ZLIB_Decompress_Fast(benchmark::State& state) {
    int level = Z_BEST_SPEED;
    uLongf bound = compressBound(static_cast<uLong>(g_input.size()));
    std::vector<uint8_t> compressed(bound);
    uLongf out_len = bound;
    int ret = compress2(compressed.data(), &out_len,
        reinterpret_cast<const Bytef*>(g_input.data()),
        static_cast<uLong>(g_input.size()),
        level);
    if (ret != Z_OK) { state.SkipWithError("zlib initial compress2 failed"); return; }
    compressed.resize(out_len);

    for (auto _ : state) {
        std::vector<uint8_t> decomp(g_input.size());
        uLongf decomp_len = static_cast<uLong>(decomp.size());
        int r = uncompress(reinterpret_cast<Bytef*>(decomp.data()), &decomp_len,
            compressed.data(), static_cast<uLong>(compressed.size()));
        if (r != Z_OK) state.SkipWithError("zlib uncompress failed");

        benchmark::DoNotOptimize(decomp);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_ZLIB_Decompress_High(benchmark::State& state) {
    int level = Z_BEST_COMPRESSION;
    uLongf bound = compressBound(static_cast<uLong>(g_input.size()));
    std::vector<uint8_t> compressed(bound);
    uLongf out_len = bound;
    int ret = compress2(compressed.data(), &out_len,
        reinterpret_cast<const Bytef*>(g_input.data()),
        static_cast<uLong>(g_input.size()),
        level);
    if (ret != Z_OK) { state.SkipWithError("zlib initial compress2 failed"); return; }
    compressed.resize(out_len);

    for (auto _ : state) {
        std::vector<uint8_t> decomp(g_input.size());
        uLongf decomp_len = static_cast<uLong>(decomp.size());
        int r = uncompress(reinterpret_cast<Bytef*>(decomp.data()), &decomp_len,
            compressed.data(), static_cast<uLong>(compressed.size()));
        if (r != Z_OK) state.SkipWithError("zlib uncompress failed");

        benchmark::DoNotOptimize(decomp);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

BENCHMARK(BM_ZLIB_Compress_Fast);
BENCHMARK(BM_ZLIB_Compress_High);
BENCHMARK(BM_ZLIB_Decompress_Fast);
BENCHMARK(BM_ZLIB_Decompress_High);


