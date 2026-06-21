#include "TitleScreen.hpp"

using namespace std;


TitleScreen::TitleScreen()
{
    reset();
}

TitleScreen::~TitleScreen()
{
}

void TitleScreen::reset()
{
    PlayerCount = 0;

    VerticesPointer = (VerticesFrameStart * VerticesOffsetSize);

    FramePointer = VerticesFrameStart;
    FramePointerDirection = 1; // 1: up, -1: down

    CoordXOffset = CoordXOffsetStart;
    CoordXOffsetDirection = 1; // 1: right, -1: left
}

void TitleScreen::printHeader()
{
    renderText(1, "   PONG-Game");
}

void TitleScreen::render()
{
    //- invert last frame lines
    drawCubeLines(LastCoordinates, true);

    //- draw next cube frame (lines)
    calcCubeCoords();

    //- transmit buffer
    showBuffer();

    //- process frame pointer / x-axis movement
    processMovement();
}

void TitleScreen::calcCubeCoords()
{
    vector<VertexScreeenCoordinate> Coordinates;

    int CoordsIndex = VerticesPointer;

    for (int i : CubeCoordIndexes) {
        VertexScreeenCoordinate PixelCoords;

        PixelCoords.xPos = CubeCoordinates[CoordsIndex]+CoordXOffset;
        PixelCoords.yPos = CubeCoordinates[CoordsIndex+1];

        Coordinates.push_back(PixelCoords);
        CoordsIndex += 2;
    }

    drawCubeLines(Coordinates);

    LastCoordinates = std::move(Coordinates);
}

void TitleScreen::drawCubeLines(std::vector<VertexScreeenCoordinate> Coordinates, bool invert)
{
    if (Coordinates.size() == 8) {
        for (int i : CubeLineMatrixIndexes) {

            int CoordIndexStart = LineDrawMatrix[i].IndexStart;
            int CoordIndexEnd = LineDrawMatrix[i].IndexEnd;

            drawLine(
                {
                    Coordinates[CoordIndexStart].xPos,
                    Coordinates[CoordIndexStart].yPos,
                    Coordinates[CoordIndexEnd].xPos,
                    Coordinates[CoordIndexEnd].yPos
                },
                invert
            );
        }
    }
}

void TitleScreen::processMovement()
{
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
}

void TitleScreen::addPlayer()
{
    if (PlayerCount < 2) {
        PlayerCount += 1;
    }
}

void TitleScreen::removePlayer()
{
    if (PlayerCount > 0) {
        PlayerCount -= 1;
    }
}

void TitleScreen::printPlayerInfo()
{
    if (PlayerCount == 0) {
        renderText(7, "P1:AI       P2:-");
    }
    else if (PlayerCount == 1) {
        string BaseString = "P1:AI       P2:";
        const char WifiCharIndex[1] = { 3 };
        renderText(7, BaseString.append(WifiCharIndex));
    }
    else if (PlayerCount == 2) {
        char WifiCharIndex[1] = { 3 };
        string BaseStringPre = "P1:";
        string BaseStringPost = "        P2:";
        string RenderString = BaseStringPre.append(WifiCharIndex).append(BaseStringPost).append(WifiCharIndex);
        renderText(7, RenderString);
    }
}

uint8_t TitleScreen::getPlayerCount()
{
        return PlayerCount;
}
