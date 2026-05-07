#pragma once

#include <cstring>

extern "C" {
#include "constants.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"

static EventGroupHandle_t InitWifiEventHandle;

static void WIFIEventHandler(
    void *Arguments,
    esp_event_base_t EventBase,
    int32_t EventId,
    void *EventData
){
    if (EventBase == WIFI_EVENT && EventId == WIFI_EVENT_AP_STACONNECTED) {
        xEventGroupSetBits(InitWifiEventHandle, WIFI_CONNECTED_BIT);
    } else if (EventBase == WIFI_EVENT && EventId == WIFI_EVENT_AP_STADISCONNECTED) {
        xEventGroupSetBits(InitWifiEventHandle, WIFI_DISCONNECTED_BIT);
    } else if (EventBase == IP_EVENT && EventId == IP_EVENT_ASSIGNED_IP_TO_CLIENT) {
        xEventGroupSetBits(InitWifiEventHandle, WIFI_STA_IP_ASSIGNED_BIT);
    }
}
}

class NetworkWifi
{

private:

    static esp_netif_t* APNetInterface;

public:

    static void initSoftAP(void)
    {
        APNetInterface = esp_netif_create_default_wifi_ap();

        wifi_config_t APConfig;
        strncpy((char*)APConfig.ap.ssid, WIFI_AP_SSID, strlen(WIFI_AP_SSID));

        if (strlen(WIFI_AP_PASSWD) == 0) {
            APConfig.ap.authmode = WIFI_AUTH_OPEN;
        }
        else {
            strncpy((char*)APConfig.ap.password, WIFI_AP_PASSWD, strlen(WIFI_AP_PASSWD));
        }

        APConfig.ap.channel = WIFI_CHANNEL;
        APConfig.ap.max_connection = WIFI_MAX_STA_CONN;
        APConfig.ap.authmode = WIFI_AUTH_WPA3_PSK;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &APConfig));
    }
};
