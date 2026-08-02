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
./run_dns_tests.sh          # DNS resolver test build
```

The script:
1. Configures and builds `server_test` and `test_client` under `linux_server/build/`.
2. Starts `server_test` on `127.0.0.1:8080` in the background.
3. Runs `test_client` which reports pass/fail per scenario.
4. Kills the server and exits with the test client's exit code.

The DNS resolver script:
1. Configures a full Linux server build under `linux_server/build-dns/`.
2. Starts `dns_server_linux` on `127.0.0.1:53535`.
3. Verifies that `pong.game` resolves to the configured IPv4 address.
4. Verifies that a different hostname is ignored.

## Manual build

```bash
cd ..
cmake -B build
cmake --build build --parallel

# Terminal 1 – start the server
./build/test/server_test 127.0.0.1

# Terminal 2 – run the tests
./build/test/test_client 127.0.0.1 8080

# DNS resolver test
./test/run_dns_tests.sh
```

## Test file set

The generated files are served at `/testfile001.html` onward using the shared
defaults from `generate_test_headers.py`. Their paths and metadata live in
generated `filemetadata-test.h` / `filedata-test.h` headers, created during the
parent `linux_server/CMakeLists.txt` configuration in `build/generated_fs/` and
refreshed again during builds when the generator changes. When tests are
enabled, CMake copies the original `/src/components/filesystem/Filesystem.hpp`
into `linux_server/filesystem/Filesystem.hpp` and rewrites only its two include
lines to point at the generated test headers, so the Linux test server serves
only the generated test files without duplicating the original filesystem
implementation. The generator defaults drive the shared generated headers, so
the served files and the client expectations always stay aligned. The test
client includes the same generated
`filedata-test.h` / `filemetadata-test.h` headers as the Linux test server and
verifies those bytes directly (files ≤ 128 KB) or spot-checks the first and
last 512 bytes (larger files). All generated test files are ASCII-only and
served with
`Content-Type: text/html`.

## Test scenarios

| # | Scenario | Coverage |
|---|----------|----------|
| 1 | Single GET per new connection × all generated files | Basic correctness, connection teardown |
| 2 | 50 sequential GETs on one keep-alive connection | Persistent connection, request queue |
| 3 | Pipelined GETs (8 requests sent before reading) × 5 bursts | HTTP pipelining, request queue depth |
| 4 | Partial-send: request split across 2 writes with 50 ms gap × 20 | Parser buffering, partial TCP segments |
| 5 | Connection closed mid-response × 10 large files | EPIPE / ECONNRESET handling |
| 6 | Stress: 500 sequential requests, 50 per connection | Throughput, memory stability |
| 7 | GET for unknown URL → 404 | 404 path |
| 8 | First 10 (small) and last 10 (large) files | Size extremes |
