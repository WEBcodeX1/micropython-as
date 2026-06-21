#pragma once

// AP configuration
#define WIFI_AP_SSID                                CONFIG_ESP_WIFI_AP_SSID
#define WIFI_AP_PASSWD                              CONFIG_ESP_WIFI_AP_PASSWORD
#define WIFI_CHANNEL                                CONFIG_ESP_WIFI_AP_CHANNEL
#define WIFI_MAX_STA_CONN                           CONFIG_ESP_MAX_STA_CONN_AP

// AP status configuration
#define WIFI_CONNECTED_BIT                          BIT0
#define WIFI_DISCONNECTED_BIT                       BIT1
#define WIFI_STA_IP_ASSIGNED_BIT                    BIT2

// MicroPython
#define MICROPYTHON_HEAP_SIZE                       CONFIG_MICROPYTHON_HEAP_SIZE

// LED flash types
#define LED_FLASH_WIFI_CONNECT                      (1)
#define LED_FLASH_WIFI_DISCONNECT                   (2)
#define LED_FLASH_PLAYER_SCORE                      (3)

//- game screen types
enum ScreenType {
    MAIN = 1,
    GAME = 2
};
