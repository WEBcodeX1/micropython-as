// main include headers
#include "Network.hpp"
#include "TitleScreen.hpp"
#include "GameScreen.hpp"
#include "NetworkWifi.hpp"
#include "Micropython.hpp"
#include "ClientHandler.hpp"

// LED control
#include "LED3Color.hpp"

// include constants
#include "constants.h"

// include PONG MicroPython .mpy object code
#include "pong.h"

// include posix thread wrapper
#include <pthread.h>
#include "esp_pthread.h"

// include rtos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// thread function prototypes
static void* led_flashing_thread(void * arg);
static void* http_server_thread(void * arg);

// set network config
bool Network::StaticIP = true;
std::string Network::IPAddr = "192.168.10.254";
std::string Network::IPGateway = "192.168.10.254";
std::string Network::IPNetmask = "255.255.255.0";

// global shared LED flash indicator
static unsigned int LEDFlashTrigger = 0;

// global shared http application server "lock"
static unsigned int HTTPAppServerMsgReady = 0;


// main loop
extern "C" void app_main(void)
{
    //- MicroPython heap
    static char InterpreterHeap[MICROPYTHON_HEAP_SIZE];

    //- LED processing "RTOS task" setup
    pthread_attr_t ThreadAttributes;
    pthread_t LEDThread;

    pthread_create(&LEDThread, NULL, led_flashing_thread, NULL);

    //- WiFi setup
    Network::init();
    NetworkWifi::registerEventHandler();
    NetworkWifi::initDefaultConfig();
    NetworkWifi::initSoftAP();

    int InterpreterStackTop;
    MicroPython interpreter(&InterpreterHeap[0], MICROPYTHON_HEAP_SIZE, &InterpreterStackTop);

    //mp_embed_exec_mpy(pong_mpy_code, pong_mpy_code_size);
    mp_embed_exec_str(pong_code1);
    mp_embed_exec_str(pong_code2);

    string ResultString;
    bool ResultStatus;

    string FunctionParamStart("{ \"start\": \"single\" }");
    string FunctionParamMove("{ \"player2\": \"up\" }");

    string FunctionName("render_frame_no_dt");


    TitleScreen TitleScreenRef;

    GameScreen GameScreenRef;
    GameScreenRef.setDisplayDev(TitleScreenRef.getDisplayDev());
    GameScreenRef.setLEDFlashTriggerRef(&LEDFlashTrigger);

    int LoopIncrementCount = 0;
    enum ScreenType SelectedScreen = MAIN;
    bool ScreenInit = true;
    bool GameRunning = false;

    while (1) {

        if (SelectedScreen == MAIN && ScreenInit == true) {
            TitleScreenRef.printHeader();
            ScreenInit = false;
        }
        else if (SelectedScreen == MAIN && ScreenInit == false) {
            TitleScreenRef.printPlayerInfo();
            TitleScreenRef.render();
        }
        else if (SelectedScreen == GAME && ScreenInit == true) {
            ResultStatus = interpreter.callFunction(
                FunctionName, FunctionParamStart, ResultString
            );
            GameScreenRef.reset();
            ScreenInit = false;
        }
        else if (SelectedScreen == GAME && ScreenInit == false) {
            ResultStatus = interpreter.callFunction(
                FunctionName, FunctionParamMove, ResultString
            );
            GameScreenRef.render(ResultString);
        }

        LoopIncrementCount++;

        if (LoopIncrementCount == 50) {

            LoopIncrementCount = 0;

            //- (temp) trigger game start
            if (TitleScreenRef.getPlayerCount() == 2 && GameRunning == false) {
                SelectedScreen = GAME;
                ScreenInit = true;
                GameRunning = true;
            }

            //- (temp) trigger game stop
            if (SelectedScreen == GAME && TitleScreenRef.getPlayerCount() == 0 && GameRunning == true) {
                SelectedScreen = MAIN;
                ScreenInit = true;
                GameRunning = false;
            }

            const EventGroupHandle_t EventHandle = getWifiEventHandle();

            EventBits_t WifiStatusBits = xEventGroupGetBits(EventHandle);

            if ( (WifiStatusBits & WIFI_CONNECTED_BIT) != 0 &&
                 (WifiStatusBits & WIFI_STA_IP_ASSIGNED_BIT) != 0)
            {
                LEDFlashTrigger = 1;
                TitleScreenRef.addPlayer();
                xEventGroupClearBits(EventHandle, WIFI_CONNECTED_BIT | WIFI_STA_IP_ASSIGNED_BIT);
            }
            if ((WifiStatusBits & WIFI_DISCONNECTED_BIT) != 0) {
                LEDFlashTrigger = 2;
                TitleScreenRef.removePlayer();
                xEventGroupClearBits(EventHandle, WIFI_DISCONNECTED_BIT);
            }
        }

        vTaskDelay(1);
    }
}

static void* led_flashing_thread(void * arg)
{
    LED3Color led1 = LED3Color();
    led1.init(GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3);
    led1.setColor(100, 0, 4000);

    while(1) {

        if (LEDFlashTrigger == LED_FLASH_WIFI_CONNECT) {
            led1.fade(
                { 100, 200, 250, 90, 100, 0, 40, 3 }
            );
            led1.fade(
                { 100, 200, 250, 90, 100, 0, 40, 3 }
            );
            LEDFlashTrigger = 0;
        }
        else if (LEDFlashTrigger == LED_FLASH_WIFI_DISCONNECT) {
            led1.fade(
                { 3600, 50, 1000, -80, 0, 20, 40, 3 }
            );
            led1.fade(
                { 3600, 50, 1000, -80, 0, 20, 40, 3 }
            );
            LEDFlashTrigger = 0;
        }
        else if (LEDFlashTrigger == LED_FLASH_PLAYER_SCORE) {
            led1.fade(
                { 100, 50, 100, 127, 0, 50, 15, 2 }
            );
            led1.fade(
                { 4100, 50, 1500, -80, 0, -10, 15, 2 }
            );
            led1.setColor(0, 0, 0);
            LEDFlashTrigger = 0;
        }
        else {
            vTaskDelay(10);
        }

        ESP_LOGI("LEDControl", "LEDFlashTrigger:%d", LEDFlashTrigger);
    }
}

static void* http_server_thread(void * arg)
{
    /*
    uint16_t fd = 1;
    ClientHandler testhandler;
    testhandler.addClient(fd);
    */
}
