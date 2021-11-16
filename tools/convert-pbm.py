#!/usr/bin/env python3

import argparse
from pathlib import Path
from itertools import zip_longest


def chunks(n, iterable, fillvalue=None):
    args = [iter(iterable)] * n
    return zip_longest(fillvalue=fillvalue, *args)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("pbm", type=Path)
    args = parser.parse_args()

    with args.pbm.open("rb") as f:
        data = f.read()
        len = len(data)
        print(f"const PROGMEM uint8_t foo[{len}] = {{")

        for chunks in chunks(8, data):
            print("  " + ", ".join((f"0x{byte:02x}" for byte in chunks)) + ",")

        print("};")

