#!/usr/bin/env python3
"""Expand a 1-byte Meshtastic PSK shorthand into a 16-byte AES key."""

import argparse
import base64
import sys

# 16-byte default PSK from src/mesh/Channels.h
DEFAULT_PSK = bytes(
    [0xD4, 0xF1, 0xBB, 0x3A, 0x20, 0x29, 0x07, 0x59, 0xF0, 0xBC, 0xFF, 0xAB, 0xCF, 0x4E, 0x69, 0x01]
)


def expand_single_byte(psk_byte: int) -> bytes:
    """
    Expand the single-byte PSK to the 16-byte AES key Meshtastic uses.
    psk_byte == 0 disables encryption and returns an empty bytes object.
    """
    if not 0 <= psk_byte <= 0xFF:
        raise ValueError("PSK must fit in one byte")
    if psk_byte == 0:
        return b""

    key = bytearray(DEFAULT_PSK)
    key[-1] = (key[-1] + psk_byte - 1) & 0xFF  # increment last byte per index, wrapping on overflow
    return bytes(key)


def parse_single_byte(text: str) -> int:
    """Parse decimal, hex (0xNN), or base64-encoded single-byte strings."""
    try:
        value = int(text, 0)
        if 0 <= value <= 0xFF:
            return value
    except ValueError:
        pass

    try:
        decoded = base64.b64decode(text, validate=True)
        if len(decoded) == 1:
            return decoded[0]
    except Exception:
        pass

    raise argparse.ArgumentTypeError(f"cannot parse single-byte PSK from '{text}'")


def print_key(psk_byte: int) -> None:
    key = expand_single_byte(psk_byte)
    if not key:
        print("PSK value 0 disables encryption; no AES key generated.")
        return
    print(f"psk byte: {psk_byte} (0x{psk_byte:02X})")
    print(f"aes-128 key hex: {key.hex()}")
    print(f"aes-128 key base64: {base64.b64encode(key).decode()}")


def print_all() -> None:
    print("idx dec hex  base64 aes-key")
    for psk_byte in range(1, 256):
        key = expand_single_byte(psk_byte)
        b64 = base64.b64encode(key).decode()
        print(f"{psk_byte:3d} 0x{psk_byte:02X} {b64} {key.hex()}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "psk",
        nargs="?",
        help="Single-byte PSK (decimal like 5, hex like 0x05, or base64 like AQ==).",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Print the expanded AES keys for PSK values 1..255.",
    )
    args = parser.parse_args(argv)

    if args.all:
        print_all()
        return 0

    if args.psk is None:
        parser.error("psk is required unless --all is used")
    psk_byte = parse_single_byte(args.psk)
    print_key(psk_byte)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
