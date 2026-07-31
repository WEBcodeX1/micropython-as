# Linux Server Stability Tests

Exercises the HTTP server component with a range of HTTP/1.1 request patterns
that are known to expose timeout-related crashes and edge-case bugs.

## Prerequisites

The same Linux-native FalconAS HTTP libraries used by the main `linux_server`
build are required (see `BUILD.md §10.1`).

## Quick start

```bash
cd linux_server/test
./run_tests.sh              # normal build
./run_tests.sh --asan       # AddressSanitizer build
./run_tests.sh --tsan       # ThreadSanitizer build
```

The script:
1. Configures and builds `server_test` and `test_client` under `test/build/`.
2. Starts `server_test` on `127.0.0.1:8080` in the background.
3. Runs `test_client` which reports pass/fail per scenario.
4. Kills the server and exits with the test client's exit code.

## Manual build

```bash
cmake -B build
cmake --build build --parallel

# Terminal 1 – start the server
./build/server_test 127.0.0.1

# Terminal 2 – run the tests
./build/test_client 127.0.0.1 8080
```

## Test file set

200 files served at `/testfile001.html` … `/testfile200.html`, linearly sized
from **10 KB** (file 1) to **1 MB** (file 200).  Their paths and metadata live
in generated `filemetadata-test.h` / `filedata-test.h` headers, created during
`cmake` configuration in `test/build/generated_fs/` and refreshed again during
builds when the generator changes. Those headers append the extra test
entries after the production `f66` entry while leaving `/src/components/filesystem/`
untouched. The test client independently recomputes and verifies the expected
bytes (files ≤ 128 KB) or spot-checks the first and last 512 bytes (larger
files). All generated test files are served with `Content-Type: text/html`.

## Test scenarios

| # | Scenario | Coverage |
|---|----------|----------|
| 1 | Single GET per new connection × 200 files | Basic correctness, connection teardown |
| 2 | 50 sequential GETs on one keep-alive connection | Persistent connection, request queue |
| 3 | Pipelined GETs (8 requests sent before reading) × 5 bursts | HTTP pipelining, request queue depth |
| 4 | Partial-send: request split across 2 writes with 50 ms gap × 20 | Parser buffering, partial TCP segments |
| 5 | Connection closed mid-response × 10 large files | EPIPE / ECONNRESET handling |
| 6 | Stress: 500 sequential requests, 50 per connection | Throughput, memory stability |
| 7 | GET for unknown URL → 404 | 404 path |
| 8 | First 10 (small) and last 10 (large) files | Size extremes |
