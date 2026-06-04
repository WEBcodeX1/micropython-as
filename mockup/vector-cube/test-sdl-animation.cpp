#include <iostream>
#include <SDL2/SDL.h>
#include "../../src/components/peripherals/metadata/cube-animation.h"

using namespace std;

static const int WindowWidth = 128;
static const int WindowHeight = 64;

static const int VerticesOffsetSize = 8*3;
static const int VerticesFrameStart = 1;

static const int CubeCoordIndexes[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

static int VerticesPointer = (VerticesFrameStart * VerticesOffsetSize);

static vector<VertexScreeenCoordinate> LastCoordinates;


int main() {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
        cout << "Initialization failed" << endl;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Vector Cube Animation",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WindowWidth,
        WindowHeight,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        SDL_Quit();
        return 2;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        0,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    bool quit = false;
    SDL_Event event;

    SDL_RenderClear(renderer);

    while (!quit) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

        for (auto PixelCoords : LastCoordinates) {
            SDL_RenderDrawPoint(renderer, PixelCoords.xPos, PixelCoords.yPos);
        }

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

        vector<VertexScreeenCoordinate> Coordinates;

        for (int i : CubeCoordIndexes) {
            VertexScreeenCoordinate PixelCoords;

            PixelCoords.xPos = CubeCoordinates[VerticesPointer];
            PixelCoords.yPos = CubeCoordinates[VerticesPointer+1];
            PixelCoords.visible = CubeCoordinates[VerticesPointer+2];

            Coordinates.push_back(PixelCoords);
            VerticesPointer += 3;

            cout << "Set coords x:" << PixelCoords.xPos << "y:" << PixelCoords.yPos << endl;
            SDL_RenderDrawPoint(renderer, PixelCoords.xPos, PixelCoords.yPos);
        }

        LastCoordinates = std::move(Coordinates);

        //c1LineData = LineDrawMatrix[0];

        SDL_RenderPresent(renderer);

        SDL_Delay(20);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}
