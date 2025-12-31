#!/usr/bin/env python3
"""Compute the Meshtastic channel hash for a given name and PSK."""

import argparse
import base64
import binascii
from typing import ByteString


DEFAULT_PSK = bytes(
    [0xD4, 0xF1, 0xBB, 0x3A, 0x20, 0x29, 0x07, 0x59, 0xF0, 0xBC, 0xFF, 0xAB, 0xCF, 0x4E, 0x69, 0x01]
)


def xor_bytes(data: ByteString) -> int:
    value = 0
    for byte in data:
        value ^= byte
    return value


def expand_psk(psk: bytes) -> bytes:
    """Mirror firmware logic for handling short PSKs."""

    if not psk:
        return b""  # encryption disabled
    if len(psk) == 1:
        index = psk[0]
        if index == 0:
            return b""
        expanded = bytearray(DEFAULT_PSK)
        expanded[-1] = (expanded[-1] + index - 1) & 0xFF
        return bytes(expanded)
    if len(psk) < 16:
        return psk + b"\x00" * (16 - len(psk))
    if len(psk) < 32 and len(psk) != 16:
        return psk + b"\x00" * (32 - len(psk))
    return psk


def decode_psk(psk: str, fmt: str) -> bytes:
    if fmt == "base64":
        return base64.b64decode(psk, validate=True)
    if fmt == "hex":
        psk = psk[2:] if psk.startswith("0x") else psk
        return binascii.unhexlify(psk)
    if fmt == "ascii":
        return psk.encode("utf-8")
    raise ValueError(f"Unsupported psk format '{fmt}'")


def compute_hash(name: str, psk: bytes) -> int:
    name_xor = xor_bytes(name.encode("utf-8"))
    psk_xor = xor_bytes(expand_psk(psk))
    return name_xor ^ psk_xor


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("name", help="Channel name")
    parser.add_argument("psk", help="Channel PSK in the selected format")
    parser.add_argument(
        "--psk-format",
        choices=("base64", "hex", "ascii"),
        default="base64",
        help="Encoding used for the PSK input (default: base64)",
    )
    args = parser.parse_args()

    raw_psk = decode_psk(args.psk, args.psk_format)
    hash_value = compute_hash(args.name, raw_psk)

    print(f"Channel hash: 0x{hash_value:02X} ({hash_value})")


if __name__ == "__main__":
    main()
