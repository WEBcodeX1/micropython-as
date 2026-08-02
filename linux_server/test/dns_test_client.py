#!/usr/bin/env python3

import socket
import struct
import sys
import time


PONG_GAME_NAME = b"\x04pong\x04game\x00"
OTHER_GAME_NAME = b"\x05other\x04game\x00"
EDNS0_OPT_RECORD = b"\x00\x00\x29\x10\x00\x00\x00\x00\x00\x00\x00"
PONG_GAME_ANSWER_OFFSET = 12 + len(PONG_GAME_NAME) + 4


def build_query(hostname: str, with_edns0: bool = False) -> bytes:
    transaction_id = 0x1337
    flags = 0x0100
    if hostname == "pong.game":
        encoded_name = PONG_GAME_NAME
    elif hostname == "other.game":
        encoded_name = OTHER_GAME_NAME
    else:
        raise ValueError(f"unsupported hostname: {hostname}")

    additional_count = 1 if with_edns0 else 0
    question = encoded_name + struct.pack("!HH", 1, 1)
    request = struct.pack("!HHHHHH", transaction_id, flags, 1, 0, 0, additional_count) + question
    if with_edns0:
        request += EDNS0_OPT_RECORD
    return request


def query_server(server_host: str, server_port: int, hostname: str, timeout: float, with_edns0: bool = False):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    request = build_query(hostname, with_edns0)
    try:
        sock.settimeout(timeout)
        sock.sendto(request, (server_host, server_port))
        response, _ = sock.recvfrom(512)
        return request, response
    except socket.timeout:
        return request, None
    finally:
        sock.close()


def expect_pong_game(server_host: str, server_port: int, expected_ip: str, with_edns0: bool = False) -> bool:
    packed_ip = socket.inet_aton(expected_ip)

    for _ in range(20):
        request, response = query_server(server_host, server_port, "pong.game", 0.25, with_edns0)
        if response is None:
            time.sleep(0.1)
            continue

        expected_length = len(request) + 16
        if len(response) != expected_length:
            print("unexpected response length", file=sys.stderr)
            return False

        if response[:2] != request[:2]:
            print("transaction id mismatch", file=sys.stderr)
            return False

        answer_count = struct.unpack("!H", response[6:8])[0]
        if answer_count != 1:
            print("unexpected answer count", file=sys.stderr)
            return False

        additional_count = struct.unpack("!H", response[10:12])[0]
        if additional_count != (1 if with_edns0 else 0):
            print("unexpected additional record count", file=sys.stderr)
            return False

        response_code = struct.unpack("!H", response[2:4])[0] & 0x000F
        if response_code != 0:
            print("unexpected response code", file=sys.stderr)
            return False

        if response[PONG_GAME_ANSWER_OFFSET + 12:PONG_GAME_ANSWER_OFFSET + 16] != packed_ip:
            print("unexpected IPv4 answer", file=sys.stderr)
            return False

        if with_edns0 and response[PONG_GAME_ANSWER_OFFSET + 16:PONG_GAME_ANSWER_OFFSET + 27] != EDNS0_OPT_RECORD:
            print("missing EDNS0 OPT record", file=sys.stderr)
            return False

        return True

    print("pong.game did not resolve", file=sys.stderr)
    return False


def expect_nxdomain(server_host: str, server_port: int, with_edns0: bool = False) -> bool:
    request, response = query_server(server_host, server_port, "other.game", 0.5, with_edns0)
    if response is None:
        print("missing NXDOMAIN response", file=sys.stderr)
        return False

    if response[:2] != request[:2]:
        print("transaction id mismatch", file=sys.stderr)
        return False

    if len(response) != len(request):
        print("unexpected NXDOMAIN response length", file=sys.stderr)
        return False

    answer_count = struct.unpack("!H", response[6:8])[0]
    if answer_count != 0:
        print("unexpected NXDOMAIN answer count", file=sys.stderr)
        return False

    additional_count = struct.unpack("!H", response[10:12])[0]
    if additional_count != (1 if with_edns0 else 0):
        print("unexpected NXDOMAIN additional record count", file=sys.stderr)
        return False

    response_code = struct.unpack("!H", response[2:4])[0] & 0x000F
    if response_code != 3:
        print("expected NXDOMAIN response code", file=sys.stderr)
        return False

    return True


def main() -> int:
    if len(sys.argv) != 4:
        print(f"usage: {sys.argv[0]} <server-host> <server-port> <expected-ip>", file=sys.stderr)
        return 2

    server_host = sys.argv[1]
    server_port = int(sys.argv[2])
    expected_ip = sys.argv[3]

    if not expect_pong_game(server_host, server_port, expected_ip):
        return 1

    if not expect_pong_game(server_host, server_port, expected_ip, with_edns0=True):
        return 1

    if not expect_nxdomain(server_host, server_port):
        return 1

    if not expect_nxdomain(server_host, server_port, with_edns0=True):
        return 1

    print("DNS resolver tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
