#pragma once

#include <string>
#include "Display.hpp"
#include "cube-animation.hpp"

using namespace std;

//- title screen constants
static const uint8_t WindowWidth = 128;
static const uint8_t WindowHeight = 64;

static const uint16_t VerticesOffsetSize = 8*2;

static const uint8_t VerticesFrameStart = 0;
static const uint8_t VerticesFrameEnd = 59;

static const uint8_t CubeCoordIndexes[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static const uint8_t CubeLineMatrixIndexes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

static int16_t CoordXOffsetStart = -50;
static int16_t CoordXOffsetEnd = 40;


class TitleScreen : public Display
{

public:

    TitleScreen();
    ~TitleScreen();

    void render();
    void printHeader();
    void addPlayer();
    void removePlayer();
    void printPlayerInfo();
    uint8_t getPlayerCount();

private:

    uint8_t PlayerCount;

    int16_t VerticesPointer;
    std::vector<VertexScreeenCoordinate> LastCoordinates;

    int16_t FramePointer;
    int8_t FramePointerDirection;

    int16_t CoordXOffset;
    int8_t CoordXOffsetDirection;

    void reset();

    void calcCubeCoords();
    void drawCubeLines(std::vector<VertexScreeenCoordinate>, bool invert=false);
    void processMovement();

};
