#!/usr/bin/env python3

import socket
import struct
import sys
import time


def encode_name(name: str) -> bytes:
    encoded = bytearray()
    for label in name.split("."):
        encoded.append(len(label))
        encoded.extend(label.encode("ascii"))
    encoded.append(0)
    return bytes(encoded)


def build_query(hostname: str) -> bytes:
    transaction_id = 0x1337
    flags = 0x0100
    question = encode_name(hostname) + struct.pack("!HH", 1, 1)
    return struct.pack("!HHHHHH", transaction_id, flags, 1, 0, 0, 0) + question


def query_server(server_host: str, server_port: int, hostname: str, timeout: float):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.settimeout(timeout)
        request = build_query(hostname)
        sock.sendto(request, (server_host, server_port))
        response, _ = sock.recvfrom(512)
        return request, response
    except socket.timeout:
        return build_query(hostname), None
    finally:
        sock.close()


def expect_pong_game(server_host: str, server_port: int, expected_ip: str) -> bool:
    packed_ip = socket.inet_aton(expected_ip)

    for _ in range(20):
        request, response = query_server(server_host, server_port, "pong.game", 0.25)
        if response is None:
            time.sleep(0.1)
            continue

        if len(response) != len(request) + 16:
            print("unexpected response length", file=sys.stderr)
            return False

        if response[:2] != request[:2]:
            print("transaction id mismatch", file=sys.stderr)
            return False

        answer_count = struct.unpack("!H", response[6:8])[0]
        if answer_count != 1:
            print("unexpected answer count", file=sys.stderr)
            return False

        if response[-4:] != packed_ip:
            print("unexpected IPv4 answer", file=sys.stderr)
            return False

        return True

    print("pong.game did not resolve", file=sys.stderr)
    return False


def expect_ignored_hostname(server_host: str, server_port: int) -> bool:
    _, response = query_server(server_host, server_port, "other.game", 0.5)
    return response is None


def main() -> int:
    if len(sys.argv) != 4:
        print(f"usage: {sys.argv[0]} <server-host> <server-port> <expected-ip>", file=sys.stderr)
        return 2

    server_host = sys.argv[1]
    server_port = int(sys.argv[2])
    expected_ip = sys.argv[3]

    if not expect_pong_game(server_host, server_port, expected_ip):
        return 1

    if not expect_ignored_hostname(server_host, server_port):
        print("unexpected response for non-pong hostname", file=sys.stderr)
        return 1

    print("DNS resolver tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
