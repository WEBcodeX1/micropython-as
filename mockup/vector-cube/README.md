# Title Screen Vector Cube Mockup

The title screen will contain a PONG puck rendered as a vector cube (animation).

## Vertices Format

The cube animations vertices have been pre-calculated using OpenGL, each animation consists of 16 integer values (8 structs containing x and y coordinates).
Initially a third value "visible" (screen z-axis) was intended to hide vertices "behind" surfaces. The calculation of this value in the pre-rendering is broken, so i decided to drop this "feature", the current vector cube rendering despite it looks real good.

## Line Drawing

All vertices will be "connected" by drawing 12 lines in total (4 x-axis, 4 y-axis and 4 z-axis).
The C++ line drawing routines have been validated using this Linux SDL mockup, if the line drawing routine included in `/src/components/peripherals/ssd1306.c` behaves like expected must be checked directly on the ESP32 device.
