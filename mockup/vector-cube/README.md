# Title Screen Vector Cube Mockup

The title screen will contain a PONG puck rendered as a vector cube
(animation).

## Compile Dependencies

The mockup uses SDL2 for rendering and requires the SDL2 headers and library.

- Debian/Ubuntu package: `libsdl2-dev`

Compile and link example:

```bash
g++ -I/usr/include/SDL2 -c -o main.o test-sdl-animation.cpp
g++ main.o -lSDL2 -o cube-animation
```

## Vertices Format

The cube animation vertices have been pre-calculated using OpenGL. Each
animation consists of 16 integer values (8 structs containing x and y
coordinates).

Initially, a third value `"visible"` (screen z-axis) was intended to hide
vertices "behind" surfaces. The calculation of this value in the pre-rendering
is broken, so I decided to drop this feature. The current vector cube rendering
still looks very good.

## Line Drawing

All vertices will be connected by drawing 12 lines in total (4 x-axis, 4 y-axis
and 4 z-axis).

The C++ line drawing routines have been validated using this Linux SDL mockup.
Whether the line drawing routine in
`/src/components/peripherals/ssd1306.c` behaves as expected must be verified
directly on the ESP32 device.
