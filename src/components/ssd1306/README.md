# Original SSD1306 Library

The SSD1306 library is taken from this GitHub repository https://github.com/nopnop2002/esp-idf-ssd1306. It has been *shrinked* to only include text, pixel and line drawing routines.

This is the modified plain C version. The main project includes C++ **wrapper** classes.

Many thanks to **nopnop2002** for the great work.

The ESP-IDF proposes using a (probably very nice) 3d library, but our intentiion is `-Os` (optimize for size, because size matters) and the nopnop2002 library is probably the smallest around.
