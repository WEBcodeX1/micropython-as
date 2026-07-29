// main include headers
#include "Server.hpp"
#include "Network.hpp"
#include "DNSServer.hpp"
#include "Filesystem.hpp"
#include "TitleScreen.hpp"
#include "GameScreen.hpp"
#include "NetworkWifi.hpp"
#include "Micropython.hpp"
#include "ASRequestHandler.hpp"
#include "ASRequestDef.hpp"
#include "DateTime.hpp"

// LED control
#include "LED3Color.hpp"

// include constants
#include "constants.h"

// include PONG MicroPython .py code
#include "pong.h"

// include posix thread wrapper
#include <pthread.h>
#include "esp_pthread.h"

// include rtos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// include extern definitions
#include "ASRequestGlobal.hpp"

// thread function prototypes
static void* led_flashing_thread(void* arg);
static void* http_server_thread(void* arg);
static void* dns_server_thread(void* arg);

// set network config
bool Network::StaticIP = true;
std::string Network::IPAddr = "192.168.10.254";
std::string Network::IPGateway = "192.168.10.254";
std::string Network::IPNetmask = "255.255.255.0";

// global shared LED flash indicator
static unsigned int LEDFlashTrigger = 0;

// as request global specs
unsigned int ASRequestStatus = AS_REQ_WAIT_IN;
unsigned int ASRequestID = 0;
unsigned int ASRequestContentLength = 0;
char ASRequestExchangeBuffer[2048];

// MicroPython game start constants
static const string MPFunctionParamStartSingle("{ \"start\": \"single\" }");
static const string MPFunctionParamStartMulti("{ \"start\": \"multi\" }");


// main loop
extern "C" void app_main(void)
{
    // set system time according to build time
    set_system_time();

    //- MicroPython heap
    static char InterpreterHeap[MICROPYTHON_HEAP_SIZE];

    //- LED processing "RTOS task" setup
    pthread_t LEDThread;

    pthread_create(&LEDThread, NULL, led_flashing_thread, NULL);
    pthread_detach(LEDThread);

    //- WiFi setup
    Network::init();
    NetworkWifi::registerEventHandler();
    NetworkWifi::initDefaultConfig();
    NetworkWifi::initSoftAP();

    //- DNS Server processing "RTOS task" setup
    pthread_t DNSServerThread;
    pthread_create(&DNSServerThread, NULL, dns_server_thread, NULL);
    pthread_detach(DNSServerThread);

    //- HTTP processing "RTOS task" setup
    esp_pthread_cfg_t esp_pthread_cfg = esp_pthread_get_default_config();
    esp_pthread_cfg.pin_to_core = 1;

    pthread_t HTTPThread;
    pthread_attr_t HTTPThreadAttributes;

    pthread_attr_init(&HTTPThreadAttributes);
    pthread_attr_setstacksize(&HTTPThreadAttributes, 16384);

    pthread_create(&HTTPThread, &HTTPThreadAttributes, http_server_thread, NULL);
    pthread_attr_destroy(&HTTPThreadAttributes);
    pthread_detach(HTTPThread);

    //- init / load MicroPython PONG code
    int InterpreterStackTop;
    MicroPython interpreter(&InterpreterHeap[0], MICROPYTHON_HEAP_SIZE, &InterpreterStackTop);

    //mp_embed_exec_mpy(pong_mpy_code, pong_mpy_code_size);
    mp_embed_exec_str(pong_code1);
    mp_embed_exec_str(pong_code2);

    string ResultString;
    bool ResultStatus;

    string MPFunctionParamStart = MPFunctionParamStartSingle;
    string MPFunctionParamMove("{ \"player2\": \"none\" }");

    string MPFunctionParamStop("{ \"quit\" }");

    string MPFunctionRenderGameFrame("render_frame_no_dt");
    string MPFunctionGetPlayer = "get_player_id";

    //- game screen data
    TitleScreen TitleScreenRef;

    GameScreen GameScreenRef;
    GameScreenRef.setDisplayDev(TitleScreenRef.getDisplayDev());
    GameScreenRef.setLEDFlashTriggerRef(&LEDFlashTrigger);

    //- init main loop
    int LoopIncrementCount = 1;
    enum ScreenType SelectedScreen = MAIN;
    bool ScreenInit = true;
    bool GameRunning = false;
    string LastProcessedPlayer = "2";

    //- loop main loop
    while (true) {

        //- server request processing
        if (ASRequestStatus == AS_REQ_PROCESSING) {

            ESP_LOGI("MainLoop", "ASRequestID:%d ASRequestStatus:%d", ASRequestID, ASRequestStatus);

            if (ASRequestID == AS_REQ_GAME_START && GameRunning == false)
            {
                if (TitleScreenRef.getPlayerCount() == 2) {
                    MPFunctionParamStart = MPFunctionParamStartMulti;
                }
                else {
                    MPFunctionParamStart = MPFunctionParamStartSingle;
                }

                SelectedScreen = GAME;
                ScreenInit = true;
                GameRunning = true;
            }

            else if (ASRequestID == AS_REQ_GAME_STOP && GameRunning == true)
            {
                ResultStatus = interpreter.callFunction(
                    MPFunctionRenderGameFrame, MPFunctionParamStop, ResultString
                );

                SelectedScreen = MAIN;
                ScreenInit = true;
                GameRunning = false;
            }

            if ((ASRequestID == AS_REQ_PADDLE_UP || ASRequestID == AS_REQ_PADDLE_DOWN) && GameRunning == true) {

                ESP_LOGI("MainLoop", "PaddleMovement ASRequestContentLength:%d", ASRequestContentLength);

                ASRequestExchangeBuffer[ASRequestContentLength+1] = 0;

                ESP_LOG_BUFFER_CHAR_LEVEL("MainLoop", &ASRequestExchangeBuffer[0], ASRequestContentLength, ESP_LOG_INFO);

                ResultStatus = interpreter.callFunctionCBuffer(
                    MPFunctionGetPlayer, &ASRequestExchangeBuffer[0], ResultString
                );

                ESP_LOGI("MainLoop", "PaddleMovement MPStatus:%d MPResult:%s", ResultStatus, ResultString.c_str());

                if (ASRequestID == AS_REQ_PADDLE_UP)
                {
                    MPFunctionParamMove = "{ \"player" + ResultString + "\": \"up\" }";
                    LastProcessedPlayer = ResultString;
                }

                else if (ASRequestID == AS_REQ_PADDLE_DOWN)
                {
                    MPFunctionParamMove = "{ \"player" + ResultString + "\": \"down\" }";
                    LastProcessedPlayer = ResultString;
                }
            }

            ASRequestStatus = AS_REQ_PROCESSED;
        }
        else {
            MPFunctionParamMove = "{ \"player" + LastProcessedPlayer + "\": \"none\" }";
        }

        //- screen rendering
        if (SelectedScreen == MAIN) {
            TitleScreenRef.render();
        }
        else if (SelectedScreen == GAME && ScreenInit == true) {
            ResultStatus = interpreter.callFunction(
                MPFunctionRenderGameFrame, MPFunctionParamStart, ResultString
            );
            GameScreenRef.reset();
            ScreenInit = false;
        }
        else if (SelectedScreen == GAME && ScreenInit == false) {
            ResultStatus = interpreter.callFunction(
                MPFunctionRenderGameFrame, MPFunctionParamMove, ResultString
            );
            GameScreenRef.render(ResultString);
        }

        //- screen rendering
        LoopIncrementCount--;

        if (LoopIncrementCount == 0) {

            LoopIncrementCount = 50;

            const EventGroupHandle_t EventHandle = getWifiEventHandle();

            EventBits_t WifiStatusBits = xEventGroupGetBits(EventHandle);

            if ( (WifiStatusBits & WIFI_CONNECTED_BIT) != 0 &&
                 (WifiStatusBits & WIFI_STA_IP_ASSIGNED_BIT) != 0)
            {
                LEDFlashTrigger = 1;
                TitleScreenRef.addPlayer();
                xEventGroupClearBits(EventHandle, WIFI_CONNECTED_BIT | WIFI_STA_IP_ASSIGNED_BIT);
            }
            else if ((WifiStatusBits & WIFI_DISCONNECTED_BIT) != 0) {
                LEDFlashTrigger = 2;
                TitleScreenRef.removePlayer();
                xEventGroupClearBits(EventHandle, WIFI_DISCONNECTED_BIT);
            }

            if (SelectedScreen == MAIN) {
                TitleScreenRef.printHeader();
                TitleScreenRef.printPlayerInfo();
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

    while(true)
    {
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
        //ESP_LOGI("LEDControl", "LEDFlashTrigger:%d", LEDFlashTrigger);
    }
    return nullptr;
}

static void* http_server_thread(void * arg)
{
    Server ServerRef;
    ServerRef.start();
    return nullptr;
}

static void* dns_server_thread(void * arg)
{
    DNSServer DNSServerRef;
    DNSServerRef.start();
    return nullptr;
}
