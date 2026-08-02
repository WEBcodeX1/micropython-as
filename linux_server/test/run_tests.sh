#!/usr/bin/env bash
# run_tests.sh – build, start the test server, run the test client, report.
#
# Usage (from linux_server/test/):
#   ./run_tests.sh [--asan] [--tsan] [test_client args...]
#
# Options:
#   --asan   Build and run with AddressSanitizer
#   --tsan   Build and run with ThreadSanitizer
#
# Exit code mirrors the test client exit code (0 = all pass).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_DIR="${SCRIPT_DIR}/.."
BUILD_DIR="${SERVER_DIR}/build"

ASAN_FLAG="OFF"
TSAN_FLAG="OFF"
TEST_ARGS=()

for arg in "$@"; do
    case "$arg" in
        --asan) ASAN_FLAG="ON" ;;
        --tsan) TSAN_FLAG="ON" ;;
        *) TEST_ARGS+=("$arg") ;;
    esac
done

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
echo "=== Configuring ==="
cmake -B "${BUILD_DIR}" -S "${SERVER_DIR}" \
      -DASAN="${ASAN_FLAG}" \
      -DTSAN="${TSAN_FLAG}"

echo "=== Building ==="
cmake --build "${BUILD_DIR}" --parallel

# ---------------------------------------------------------------------------
# Start server
# ---------------------------------------------------------------------------
SERVER_BIN="${BUILD_DIR}/test/server_test"
TEST_BIN="${BUILD_DIR}/test/test_client"
HOST="127.0.0.1"
PORT="8080"

echo ""
echo "=== Starting server_test on ${HOST}:${PORT} ==="
"${SERVER_BIN}" "${HOST}" &
SERVER_PID=$!
echo "server_test PID: ${SERVER_PID}"

# Wait for the server to bind and begin accepting
for i in {1..20}; do
    sleep 0.5
    # probe with a single byte connect; success means socket is accepting
    if bash -c "echo -n '' | nc -w1 ${HOST} ${PORT}" 2>/dev/null; then
        echo "Server ready."
        break
    fi
    if [ $i -eq 20 ]; then
        echo "ERROR: server did not start within 10 seconds" >&2
        kill "${SERVER_PID}" 2>/dev/null || true
        exit 1
    fi
done

# ---------------------------------------------------------------------------
# Run tests
# ---------------------------------------------------------------------------
echo ""
echo "=== Running test_client ==="
set +e
"${TEST_BIN}" "${HOST}" "${PORT}" "${TEST_ARGS[@]}"
TEST_RC=$?
set -e

# ---------------------------------------------------------------------------
# Stop server
# ---------------------------------------------------------------------------
echo ""
echo "=== Stopping server (PID ${SERVER_PID}) ==="
kill "${SERVER_PID}" 2>/dev/null || true
wait "${SERVER_PID}" 2>/dev/null || true

echo ""
if [ ${TEST_RC} -eq 0 ]; then
    echo "ALL TESTS PASSED"
else
    echo "SOME TESTS FAILED (exit code ${TEST_RC})" >&2
fi
exit ${TEST_RC}
