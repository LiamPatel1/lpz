#include <lzma.h>
#include <benchmark/benchmark.h>

extern std::vector<uint8_t> g_input;

static size_t lzma_guess_output_size(size_t in_size) {
    return in_size + (in_size / 3) + 128;
}

static void BM_LZMA_Compress_Fast(benchmark::State& state) {
    uint32_t preset = 0; 
    size_t last_size = 0;

    for (auto _ : state) {
        std::vector<uint8_t> compressed(lzma_guess_output_size(g_input.size()));
        lzma_stream strm = LZMA_STREAM_INIT;

        lzma_ret ret = lzma_easy_encoder(&strm, preset, LZMA_CHECK_CRC64);
        if (ret != LZMA_OK) {
            state.SkipWithError("lzma_easy_encoder failed");
            return;
        }

        strm.next_in = g_input.data();
        strm.avail_in = g_input.size();
        strm.next_out = compressed.data();
        strm.avail_out = compressed.size();

        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret != LZMA_STREAM_END && ret != LZMA_OK) {
            lzma_end(&strm);
            state.SkipWithError("lzma_code failed");
            return;
        }

        size_t cSize = compressed.size() - strm.avail_out;
        lzma_end(&strm);

        compressed.resize(cSize);
        last_size = cSize;
        benchmark::DoNotOptimize(compressed);
    }

    state.counters["Ratio"] = static_cast<double>(last_size) / g_input.size();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_LZMA_Compress_High(benchmark::State& state) {
    uint32_t preset = 9; 
    size_t last_size = 0;

    for (auto _ : state) {
        std::vector<uint8_t> compressed(lzma_guess_output_size(g_input.size()));
        lzma_stream strm = LZMA_STREAM_INIT;

        lzma_ret ret = lzma_easy_encoder(&strm, preset, LZMA_CHECK_CRC64);
        if (ret != LZMA_OK) {
            state.SkipWithError("lzma_easy_encoder failed");
            return;
        }

        strm.next_in = g_input.data();
        strm.avail_in = g_input.size();
        strm.next_out = compressed.data();
        strm.avail_out = compressed.size();

        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret != LZMA_STREAM_END && ret != LZMA_OK) {
            lzma_end(&strm);
            state.SkipWithError("lzma_code failed");
            return;
        }

        size_t cSize = compressed.size() - strm.avail_out;
        lzma_end(&strm);

        compressed.resize(cSize);
        last_size = cSize;
        benchmark::DoNotOptimize(compressed);
    }

    state.counters["Ratio"] = static_cast<double>(last_size) / g_input.size();
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_LZMA_Decompress_Fast(benchmark::State& state) {
    uint32_t preset = 0;
    std::vector<uint8_t> compressed(lzma_guess_output_size(g_input.size()));
    {
        lzma_stream enc = LZMA_STREAM_INIT;
        lzma_ret r = lzma_easy_encoder(&enc, preset, LZMA_CHECK_CRC64);
        if (r != LZMA_OK) { state.SkipWithError("lzma_easy_encoder (prepare) failed"); return; }
        enc.next_in = g_input.data();
        enc.avail_in = g_input.size();
        enc.next_out = compressed.data();
        enc.avail_out = compressed.size();
        r = lzma_code(&enc, LZMA_FINISH);
        if ((r != LZMA_STREAM_END) && (r != LZMA_OK)) { lzma_end(&enc); state.SkipWithError("lzma_code (prepare) failed"); return; }
        compressed.resize(compressed.size() - enc.avail_out);
        lzma_end(&enc);
    }

    for (auto _ : state) {
        std::vector<uint8_t> decomp(g_input.size());
        lzma_stream dec = LZMA_STREAM_INIT;
        lzma_ret r = lzma_stream_decoder(&dec, UINT64_MAX, 0);
        if (r != LZMA_OK) { state.SkipWithError("lzma_stream_decoder failed"); return; }

        dec.next_in = compressed.data();
        dec.avail_in = compressed.size();
        dec.next_out = decomp.data();
        dec.avail_out = decomp.size();

        r = lzma_code(&dec, LZMA_FINISH);
        if (r != LZMA_STREAM_END && r != LZMA_OK) {
            lzma_end(&dec);
            state.SkipWithError("lzma_code (decode) failed");
            return;
        }

        lzma_end(&dec);
        benchmark::DoNotOptimize(decomp);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

static void BM_LZMA_Decompress_High(benchmark::State& state) {
    uint32_t preset = 9;
    std::vector<uint8_t> compressed(lzma_guess_output_size(g_input.size()));
    {
        lzma_stream enc = LZMA_STREAM_INIT;
        lzma_ret r = lzma_easy_encoder(&enc, preset, LZMA_CHECK_CRC64);
        if (r != LZMA_OK) { state.SkipWithError("lzma_easy_encoder (prepare) failed"); return; }
        enc.next_in = g_input.data();
        enc.avail_in = g_input.size();
        enc.next_out = compressed.data();
        enc.avail_out = compressed.size();
        r = lzma_code(&enc, LZMA_FINISH);
        if ((r != LZMA_STREAM_END) && (r != LZMA_OK)) { lzma_end(&enc); state.SkipWithError("lzma_code (prepare) failed"); return; }
        compressed.resize(compressed.size() - enc.avail_out);
        lzma_end(&enc);
    }

    for (auto _ : state) {
        std::vector<uint8_t> decomp(g_input.size());
        lzma_stream dec = LZMA_STREAM_INIT;
        lzma_ret r = lzma_stream_decoder(&dec, UINT64_MAX, 0);
        if (r != LZMA_OK) { state.SkipWithError("lzma_stream_decoder failed"); return; }

        dec.next_in = compressed.data();
        dec.avail_in = compressed.size();
        dec.next_out = decomp.data();
        dec.avail_out = decomp.size();

        r = lzma_code(&dec, LZMA_FINISH);
        if (r != LZMA_STREAM_END && r != LZMA_OK) {
            lzma_end(&dec);
            state.SkipWithError("lzma_code (decode) failed");
            return;
        }

        lzma_end(&dec);
        benchmark::DoNotOptimize(decomp);
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(g_input.size()));
}

BENCHMARK(BM_LZMA_Compress_Fast);
BENCHMARK(BM_LZMA_Compress_High);
BENCHMARK(BM_LZMA_Decompress_Fast);
BENCHMARK(BM_LZMA_Decompress_High);
