#!/bin/sh
g++ -I/usr/include/SDL2 -c -o main.o test-sdl-animation.cpp
g++ main.o -lSDL2 -o cube-animation 
