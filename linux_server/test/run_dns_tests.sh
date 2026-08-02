#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_DIR="${SCRIPT_DIR}/.."
BUILD_DIR="${SERVER_DIR}/build-dns"
DNS_BIN="${BUILD_DIR}/dns_server_linux"
CLIENT_BIN="${SCRIPT_DIR}/dns_test_client.py"
SERVER_HOST="127.0.0.1"
SERVER_IP="127.0.0.1"
SERVER_PORT="53535"

echo "=== Configuring DNS test build ==="
cmake -B "${BUILD_DIR}" -S "${SERVER_DIR}"

echo "=== Building DNS server ==="
cmake --build "${BUILD_DIR}" --parallel

echo ""
echo "=== Starting dns_server_linux on ${SERVER_HOST}:${SERVER_PORT} ==="
"${DNS_BIN}" "${SERVER_IP}" "${SERVER_PORT}" &
SERVER_PID=$!
echo "dns_server_linux PID: ${SERVER_PID}"

echo ""
echo "=== Running DNS resolver tests ==="
set +e
python3 "${CLIENT_BIN}" "${SERVER_HOST}" "${SERVER_PORT}" "${SERVER_IP}"
TEST_RC=$?
set -e

echo ""
echo "=== Stopping DNS server (PID ${SERVER_PID}) ==="
kill "${SERVER_PID}" 2>/dev/null || true
wait "${SERVER_PID}" 2>/dev/null || true

exit ${TEST_RC}
