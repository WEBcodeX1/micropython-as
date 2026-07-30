#pragma once

//- c++ header
#include <string>
#include <vector>

//- esp related header
#include "esp_log.h"

//- class internal header
#include "Display.hpp"


using namespace std;


typedef vector<string> RuntimeDataContainer;
typedef RuntimeDataContainer& RuntimeDataContainerRef;


class GameScreen : public Display
{

public:

    GameScreen();
    ~GameScreen();

    void reset();
    void setLEDFlashTriggerRef(unsigned int*);
    void render(string&);

private:

    RuntimeDataContainer RuntimeDataSplit;
    RuntimeDataContainer RuntimeDataSplitLast;

    uint16_t ScorePlayer1;
    uint16_t ScorePlayer2;

    unsigned int* LEDFlashTriggerRef = nullptr;

    void renderPuck(bool);
    void renderPaddle(uint8_t, bool);
    void renderScores();

};
