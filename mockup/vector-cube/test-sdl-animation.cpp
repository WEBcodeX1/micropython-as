#include <iostream>
#include <SDL2/SDL.h>
#include "../../src/components/peripherals/metadata/cube-animation.h"

using namespace std;

static const int WindowWidth = 128;
static const int WindowHeight = 64;

static const int VerticesOffsetSize = 8*3;

static const int VerticesFrameStart = 60;
static const int VerticesFrameEnd = 120;

static const int CubeCoordIndexes[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static const int CubeLineMatrixIndexes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

static int VerticesPointer = (VerticesFrameStart * VerticesOffsetSize);

static vector<VertexScreeenCoordinate> LastCoordinates;

static int FramePointer = VerticesFrameStart;
static int FramePointerDirection = 1; // 1: up, -1: down

static int CoordXOffsetStart = -50;
static int CoordXOffsetEnd = 40;

static int CoordXOffset = CoordXOffsetStart;
static int CoordXOffsetDirection = 1; // 1: right, -1: left


void drawLines(SDL_Renderer* renderer, vector<VertexScreeenCoordinate> Coordinates)
{
    if (Coordinates.size() == 8) {
        for (int i : CubeLineMatrixIndexes) {
            int CoordIndexStart = LineDrawMatrix[i].IndexStart;
            int CoordIndexEnd = LineDrawMatrix[i].IndexEnd;

            SDL_RenderDrawLine(
                renderer,
                Coordinates[CoordIndexStart].xPos,
                Coordinates[CoordIndexStart].yPos,
                Coordinates[CoordIndexEnd].xPos,
                Coordinates[CoordIndexEnd].yPos
            );
        }
    }
}

int main()
{
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

        drawLines(renderer, LastCoordinates);

        for (auto PixelCoords : LastCoordinates) {
            SDL_RenderDrawPoint(renderer, PixelCoords.xPos, PixelCoords.yPos);
        }

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

        vector<VertexScreeenCoordinate> Coordinates;

        int CoordsIndex = VerticesPointer;

        for (int i : CubeCoordIndexes) {
            VertexScreeenCoordinate PixelCoords;

            PixelCoords.xPos = CubeCoordinates[CoordsIndex]+CoordXOffset;
            PixelCoords.yPos = CubeCoordinates[CoordsIndex+1];
            PixelCoords.visible = CubeCoordinates[CoordsIndex+2];

            Coordinates.push_back(PixelCoords);
            CoordsIndex += 3;

            cout << "Set coords index:" << i << "x:" << PixelCoords.xPos << "y:" << PixelCoords.yPos << endl;
            SDL_RenderDrawPoint(renderer, PixelCoords.xPos, PixelCoords.yPos);
        }

        drawLines(renderer, Coordinates);

        LastCoordinates = std::move(Coordinates);

        SDL_RenderPresent(renderer);

        if (FramePointerDirection == 1) {
            VerticesPointer += VerticesOffsetSize;
            FramePointer++;
            if (FramePointer >= VerticesFrameEnd) {
                FramePointerDirection = -1;
            }
        }
        else if (FramePointerDirection == -1) {
            VerticesPointer -= VerticesOffsetSize;
            FramePointer--;
            if (FramePointer <= VerticesFrameStart) {
                FramePointerDirection = 1;
            }
        }

        if (CoordXOffsetDirection == 1) {
            CoordXOffset+=2;
            if (CoordXOffset >= CoordXOffsetEnd) {
                CoordXOffsetDirection = -1;
            }
        }
        else if (CoordXOffsetDirection == -1) {
            CoordXOffset-=2;
            if (CoordXOffset <= CoordXOffsetStart) {
                CoordXOffsetDirection = 1;
            }
        }
        //cout << "CoordXOffset:" << CoordXOffset << " Direction:" << CoordXOffsetDirection << endl;

        SDL_Delay(20);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}
