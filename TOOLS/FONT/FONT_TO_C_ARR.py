from freetype import Face

# Load font
face = Face("../../../../../Downloads/BmPlus_IBM_VGA_8x16-2x.otb")
face.set_pixel_sizes(0, 16)

# Expected glyph size (8x16 for VGA)
GLYPH_W = 8
GLYPH_H = 16

print("const U8 font[256][16] = {")

for ch in range(256):
    try:
        face.load_char(chr(ch))
        bitmap = face.glyph.bitmap
        rows, width, buffer = bitmap.rows, bitmap.width, bitmap.buffer
    except Exception:
        # Fallback if glyph is missing
        rows, width, buffer = GLYPH_H, GLYPH_W, [0] * (GLYPH_W * GLYPH_H)

    print("  {", end="")

    for y in range(GLYPH_H):
        byte = 0
        for x in range(GLYPH_W):
            # Protect against short bitmaps
            if y < rows and x < width:
                if buffer[y * width + x] > 0:
                    byte |= 1 << (7 - x)
        print(f"0x{byte:02X},", end="")
    print("},")  # end glyph

print("};")
