LPZ is a compression library written in C++.

Benchmarks and comparisons to other libraries:


| Algorithm | Level | Compression Ratio | Compression Speed (Mi/s) | Decompression Speed (Mi/s) |
| :--- | :--- | :---: | :---: | :---: |
| **LPZ** | — | 0.410 | 102.8 | 43 |
| **ZSTD** | Fast | 0.395 | 397.4 | 1240 |
| **ZSTD** | High | 0.300 | 6.0 | 1013 |
| **DEFLATE (ZLIB)** | Fast | 0.414 | 104.0 | 362 |
| **DEFLATE (ZLIB)** | High | 0.356 | 024.7 | 366 |
| **LZMA (XZ)** | Fast | 0.359 | 27.3 | 73 |
| **LZMA (XZ)** | High | 0.291 | 4.5 | 97 |