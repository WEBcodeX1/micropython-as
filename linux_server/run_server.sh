#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}"
SERVER_BIN="${BUILD_DIR}/server_linux"
PID_FILE="${BUILD_DIR}/server_linux.pid"
HOST="0.0.0.0"
LOG_ENABLED="OFF"
LOG_FILE="${BUILD_DIR}/server_linux.log"

usage() {
    cat <<EOF
Usage:
  ./run_server.sh start [--host <ipv4>] [--build-dir <dir>] [--log] [--log-file <path>]
  ./run_server.sh stop [--build-dir <dir>]
  ./run_server.sh status [--build-dir <dir>]
  ./run_server.sh restart [--host <ipv4>] [--build-dir <dir>] [--log] [--log-file <path>]

Options:
  --host <ipv4>       Listen address for server_linux (default: 0.0.0.0)
  --build-dir <dir>   Directory containing server_linux (default: ./build)
  --log               Save server stdout/stderr to a log file
  --log-file <path>   Path for the log file (default: <build-dir>/server_linux.log)
EOF
}

resolve_path() {
    local input="$1"
    if [[ "${input}" = /* ]]; then
        printf '%s\n' "${input}"
    else
        printf '%s\n' "${SCRIPT_DIR}/${input}"
    fi
}

refresh_paths() {
    BUILD_DIR="$(resolve_path "${BUILD_DIR}")"
    SERVER_BIN="${BUILD_DIR}/server_linux"
    PID_FILE="${BUILD_DIR}/server_linux.pid"
    if [[ "${LOG_FILE}" != /* ]]; then
        LOG_FILE="${BUILD_DIR}/server_linux.log"
    fi
}

is_running() {
    [[ -f "${PID_FILE}" ]] || return 1
    local pid
    pid="$(cat "${PID_FILE}")"
    [[ -n "${pid}" ]] || return 1
    kill -0 "${pid}" 2>/dev/null
}

stop_server_impl() {
    if ! is_running; then
        rm -f "${PID_FILE}"
        echo "server_linux is not running"
        return 0
    fi

    local pid
    pid="$(cat "${PID_FILE}")"
    kill "${pid}" 2>/dev/null || true

    for _ in {1..20}; do
        if ! kill -0 "${pid}" 2>/dev/null; then
            rm -f "${PID_FILE}"
            echo "Stopped server_linux (PID ${pid})"
            return 0
        fi
        sleep 0.1
    done

    echo "ERROR: failed to stop server_linux (PID ${pid})" >&2
    return 1
}

start_server() {
    refresh_paths

    if [[ ! -x "${SERVER_BIN}" ]]; then
        echo "ERROR: server binary not found or not executable: ${SERVER_BIN}" >&2
        exit 1
    fi

    mkdir -p "${BUILD_DIR}"

    if is_running; then
        echo "server_linux already running with PID $(cat "${PID_FILE}")"
        exit 0
    fi

    if [[ -f "${PID_FILE}" ]]; then
        rm -f "${PID_FILE}"
    fi

    if [[ "${LOG_ENABLED}" = "ON" ]]; then
        mkdir -p "$(dirname "${LOG_FILE}")"
        nohup "${SERVER_BIN}" "${HOST}" >>"${LOG_FILE}" 2>&1 &
    else
        nohup "${SERVER_BIN}" "${HOST}" >/dev/null 2>&1 &
    fi

    local pid=$!
    echo "${pid}" > "${PID_FILE}"
    sleep 0.2

    if kill -0 "${pid}" 2>/dev/null; then
        echo "Started server_linux on ${HOST} with PID ${pid}"
        if [[ "${LOG_ENABLED}" = "ON" ]]; then
            echo "Logging to ${LOG_FILE}"
        fi
        exit 0
    fi

    rm -f "${PID_FILE}"
    echo "ERROR: server_linux failed to stay running" >&2
    if [[ "${LOG_ENABLED}" = "ON" && -f "${LOG_FILE}" ]]; then
        echo "--- log output ---" >&2
        cat "${LOG_FILE}" >&2
    fi
    exit 1
}

stop_server() {
    refresh_paths
    stop_server_impl
}

status_server() {
    refresh_paths

    if is_running; then
        echo "server_linux is running with PID $(cat "${PID_FILE}")"
    else
        echo "server_linux is not running"
        exit 1
    fi
}

if [[ $# -lt 1 ]]; then
    usage
    exit 1
fi

COMMAND="$1"
shift

while [[ $# -gt 0 ]]; do
    case "$1" in
        --host)
            HOST="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --log)
            LOG_ENABLED="ON"
            shift
            ;;
        --log-file)
            LOG_ENABLED="ON"
            LOG_FILE="$(resolve_path "$2")"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "ERROR: unknown argument: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

case "${COMMAND}" in
    start) start_server ;;
    stop) stop_server ;;
    status) status_server ;;
    restart)
        refresh_paths
        stop_server_impl || true
        start_server
        ;;
    -h|--help)
        usage
        ;;
    *)
        echo "ERROR: unknown command: ${COMMAND}" >&2
        usage >&2
        exit 1
        ;;
esac
