#!/usr/bin/env python3
"""
font_extract.py - Extract 8x8 font glyphs from font-truncated.png and generate
                  font8x8_basic.h (C header) and font8x8.ascii (human-readable).

Image layout (128x64, 8x8 px per cell, 16 columns x 8 rows):
  Rows 0-1  : decorative header, not mapped.
  Row 2 (y=16-23): '!"#$%&'()*+,-./0'  -> U+0021 - U+0030
  Row 3 (y=24-31): '123456789:;<=>?@'  -> U+0031 - U+0040
  Row 4 (y=32-39): 'ABCDEFGHIJKLMNOP'  -> U+0041 - U+0050
  Row 5 (y=40-47): 'QRSTUVWXYZ[\\]^_ ' -> U+0051 - U+005F, col15 = U+0020 (space)
  Row 6 (y=48-55): '`abcdefghijklmno'  -> U+0060 - U+006F
  Row 7 (y=56-63): 'pqrstuvwxyz{|}~ '  -> U+0070 - U+007E, col15 = space (ignored)

SSD1306 column-major encoding:
  8 bytes per glyph; byte[c] encodes column c (c=0 is leftmost).
  Bit 0 = top pixel row, bit 7 = bottom pixel row.

Usage:
  pip install Pillow
  python3 font_extract.py [--image IMAGE] [--header HEADER] [--ascii ASCII]

Defaults (relative to this script's directory):
  IMAGE  : font-truncated.png
  HEADER : ../src/components/peripherals/font8x8_basic.h
  ASCII  : ../src/components/peripherals/font8x8.ascii
"""

import argparse
import os
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

import numpy as np

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Default paths (relative to this script)
DEFAULT_IMAGE  = os.path.join(SCRIPT_DIR, "font-truncated.png")
DEFAULT_HEADER = os.path.join(SCRIPT_DIR, "..", "src", "components", "peripherals", "font8x8_basic.h")
DEFAULT_ASCII  = os.path.join(SCRIPT_DIR, "..", "src", "components", "peripherals", "font8x8.ascii")

# Character rows: (image_row_index, char_string, ascii_start_code)
# image_row_index is 0-based; the font data starts at image row 2.
CHAR_ROWS = [
    (2, "!\"#$%&'()*+,-./0",  0x21),
    (3, "123456789:;<=>?@",   0x31),
    (4, "ABCDEFGHIJKLMNOP",   0x41),
    (5, "QRSTUVWXYZ[\\]^_ ",  0x51),   # col 15 (' ') maps to U+0020
    (6, "`abcdefghijklmno",   0x60),
    (7, "pqrstuvwxyz{|}~ ",   0x70),   # col 15 (' ') ignored (already at 0x7F)
]

# Comment labels used in the C header
CHAR_LABELS = {i: f"({chr(i)})" for i in range(0x21, 0x7F)}
CHAR_LABELS[0x20] = "(space)"
CHAR_LABELS[0x5C] = "(\\)"   # backslash


def extract_glyphs(arr):
    """
    Return (bitmap_table, art_table) for ASCII codes 0x00-0x7F.

    bitmap_table[i] : list of 8 ints (SSD1306 column-major bytes)
    art_table[i]    : list of 8 strings, each 8 chars wide ('X' / '.')
    """
    bitmap = [[0] * 8 for _ in range(128)]
    art    = [["........"] * 8 for _ in range(128)]

    for img_row, chars, ascii_start in CHAR_ROWS:
        cy = img_row * 8  # top y of cell in the image
        for col, ch in enumerate(chars):
            ascii_code = ascii_start + col
            cx = col * 8  # left x of cell

            if ch == " ":
                # space is always all-zero regardless of image content
                ascii_code = 0x20
                bitmap[ascii_code] = [0] * 8
                art[ascii_code]    = ["........"] * 8
                continue

            # SSD1306 column-major: byte[c] encodes column c
            glyph_bytes = []
            glyph_rows  = []
            for c in range(8):
                byte = 0
                row_str = ""
                for r in range(8):
                    lit = arr[cy + r, cx + c, 0] > 128
                    if lit:
                        byte |= (1 << r)
                    row_str += "X" if arr[cy + r, cx + c, 0] > 128 else "."
                glyph_bytes.append(byte)

            # Build pixel-row art strings (row-major for human reading)
            for r in range(8):
                row_str = ""
                for c in range(8):
                    row_str += "X" if arr[cy + r, cx + c, 0] > 128 else "."
                glyph_rows.append(row_str)

            bitmap[ascii_code] = glyph_bytes
            art[ascii_code]    = glyph_rows

    return bitmap, art


def write_header(path, bitmap):
    lines = [
        "#pragma once",
        "",
        "// Font extracted from image/font-truncated.png (8x8 pixel characters)",
        "// Column-major encoding: byte[col], bit0=top row, bit7=bottom row",
        "",
        "static uint8_t font8x8_basic_tr[128][8] = {",
    ]
    for i in range(128):
        b = bitmap[i]
        hex_bytes = ", ".join(f"0x{x:02X}" for x in b)
        label = CHAR_LABELS.get(i, "")
        comment = f"   // U+{i:04X} {label}" if label else f"   // U+{i:04X}"
        comma = "," if i < 127 else ""
        lines.append(f"    {{ {hex_bytes} }}{comma}{comment}")
    lines.append("};")
    lines.append("")

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write("\n".join(lines))
    print(f"Wrote {path}")


def write_ascii(path, art):
    lines = [
        "font8x8.ascii - Human-readable bitmap for each character in font8x8_basic_tr",
        "Encoding: column-major, byte[col], bit0=top row",
        "Pixel: X=lit, .=dark",
        "",
    ]
    for i in range(0x20, 0x7F):
        if i == 0x20:
            label = "space"
        elif i == 0x5C:
            label = "backslash"
        else:
            label = chr(i)
        lines.append(f"U+{i:04X} ({label})")
        lines.extend(art[i])
        lines.append("")

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write("\n".join(lines))
    print(f"Wrote {path}")


def main():
    parser = argparse.ArgumentParser(description="Extract 8x8 font glyphs from font-truncated.png")
    parser.add_argument("--image",  default=DEFAULT_IMAGE,  help="Source PNG image")
    parser.add_argument("--header", default=DEFAULT_HEADER, help="Output C header file")
    parser.add_argument("--ascii",  default=DEFAULT_ASCII,  help="Output ASCII art file")
    args = parser.parse_args()

    img = Image.open(args.image).convert("RGB")
    w, h = img.size
    if w != 128 or h != 64:
        sys.exit(f"Expected 128x64 image, got {w}x{h}: {args.image}")

    arr = np.array(img)
    bitmap, art = extract_glyphs(arr)
    write_header(args.header, bitmap)
    write_ascii(args.ascii, art)


if __name__ == "__main__":
    main()
