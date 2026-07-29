#include "GameScreen.hpp"
#include "Helper.hpp"

using namespace std;


GameScreen::GameScreen() :
    Display(false)
{
    reset();
}

GameScreen::~GameScreen()
{
}

void GameScreen::reset()
{
    ScorePlayer1 = 0;
    ScorePlayer2 = 0;
}

void GameScreen::setLEDFlashTriggerRef(unsigned int* TriggerRef)
{
    LEDFlashTriggerRef = TriggerRef;
}

void GameScreen::render(string& GameData)
{
    //- clear vector
    RuntimeDataSplit.clear();

    //- split raw game data into vector
    StringHelper::split(
        GameData,
        ",",
        RuntimeDataSplit
    );

    //- render middle line
    drawLine(
        { 64, 8, 64, 56 }, false
    );

    //- render objects
    renderPuck(true);
    renderPuck(false);

    renderPaddle(1, true);
    renderPaddle(1, false);
    renderPaddle(2, true);
    renderPaddle(2, false);

    renderScores();

    //- push runtime data
    RuntimeDataSplitLast = std::move(RuntimeDataSplit);

    //- transfer buffer to device
    showBuffer();

}

void GameScreen::renderPuck(bool clear)
{
    const RuntimeDataContainerRef DataContainer = (clear == true) ? RuntimeDataSplitLast : RuntimeDataSplit;

    if (DataContainer.size() == 6) {

        const uint8_t PuckCoordX = stoi(DataContainer[0]);
        const uint8_t PuckCoordY = stoi(DataContainer[1]);

        if (PuckCoordX >= 0 && PuckCoordX <= 127 && PuckCoordY >= 0 && PuckCoordY <= 62) {

            drawLine(
                { PuckCoordX, PuckCoordY, PuckCoordX+1, PuckCoordY },
                clear
            );

            drawLine(
                { PuckCoordX, PuckCoordY+1, PuckCoordX+2, PuckCoordY+1 },
                clear
            );
        }
    }

}

void GameScreen::renderPaddle(uint8_t Player, bool clear)
{
    const RuntimeDataContainerRef DataContainer = (clear == true) ? RuntimeDataSplitLast : RuntimeDataSplit;

    if (DataContainer.size() == 6) {

        const uint8_t PaddleYCoord = (Player == 1) ? stoi(DataContainer[2]) : stoi(DataContainer[3]);
        const uint8_t PaddleXAdd = (Player == 1) ? 0 : 120;

        if (PaddleYCoord >= 2 && PaddleYCoord <= 57) {

            const uint8_t DrawYCoord = (PaddleYCoord <= 53) ? PaddleYCoord+10 : 63;

            drawLine(
                { 4+PaddleXAdd, PaddleYCoord, 4+PaddleXAdd, DrawYCoord },
                clear
            );
            drawLine(
                { 5+PaddleXAdd, PaddleYCoord, 5+PaddleXAdd, DrawYCoord },
                clear
            );
            drawLine(
                { 6+PaddleXAdd, PaddleYCoord, 6+PaddleXAdd, DrawYCoord },
                clear
            );
        }
    }
}

void GameScreen::renderScores()
{
    if (
        (ScorePlayer1 == 0 && ScorePlayer2 == 0) ||
         ScorePlayer1 < stoi(RuntimeDataSplit[4]) ||
         ScorePlayer2 < stoi(RuntimeDataSplit[5]))
    {

        //ESP_LOGI("GameScreen", "Render scores runtime s1:%s s2:%s class s1:%d s2:%d", RuntimeDataSplit[4].c_str(), RuntimeDataSplit[5].c_str(), ScorePlayer1, ScorePlayer2);

        *LEDFlashTriggerRef = 3;

        ScorePlayer1 = stoi(RuntimeDataSplit[4]);
        ScorePlayer2 = stoi(RuntimeDataSplit[5]);

        uint8_t SpaceCount = 14;

        if (RuntimeDataSplit[4].length() > 1) {
            SpaceCount -= RuntimeDataSplit[4].length()-1;
        }

        if (RuntimeDataSplit[5].length() > 1) {
            SpaceCount -= RuntimeDataSplit[5].length()-1;
        }

        string Scores = RuntimeDataSplit[4];
        Scores.insert(RuntimeDataSplit[4].length(), SpaceCount, ' ');
        Scores.append(RuntimeDataSplit[5]);

        renderText(0, Scores);
    }
}
