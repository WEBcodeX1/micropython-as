#pragma once

#include "esp_wifi.h"

#include <cstring>

#include "constants.h"
#include "Network.hpp"


extern "C" {
#include "freertos/event_groups.h"

static EventGroupHandle_t WifiEventHandle;

static void WIFIEventHandler(
    void *Arguments,
    esp_event_base_t EventBase,
    int32_t EventId,
    void *EventData
){
    if (EventBase == WIFI_EVENT && EventId == WIFI_EVENT_AP_STACONNECTED) {
        xEventGroupSetBits(WifiEventHandle, WIFI_CONNECTED_BIT);
    } else if (EventBase == WIFI_EVENT && EventId == WIFI_EVENT_AP_STADISCONNECTED) {
        xEventGroupSetBits(WifiEventHandle, WIFI_DISCONNECTED_BIT);
    } else if (EventBase == IP_EVENT && EventId == IP_EVENT_ASSIGNED_IP_TO_CLIENT) {
        xEventGroupSetBits(WifiEventHandle, WIFI_STA_IP_ASSIGNED_BIT);
    }
}
}

class NetworkWifi
{

private:

public:

    static esp_netif_t* setupAPInterface()
    {
        return esp_netif_create_default_wifi_ap();
    }

    static void initDefaultConfig()
    {
        wifi_init_config_t APDefaultConfig = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&APDefaultConfig));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    }

    static void initSoftAP(esp_netif_t* APNetInterface)
    {
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

        if (Network::StaticIP == true) {
            Network::reconfigureDHCP(APNetInterface);
        }

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &APConfig));
    }

    static void registerEventHandler()
    {
        WifiEventHandle = xEventGroupCreate();

        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(
                WIFI_EVENT,
                ESP_EVENT_ANY_ID,
                &WIFIEventHandler,
                NULL,
                NULL
            )
        );

        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(
                IP_EVENT,
                IP_EVENT_ASSIGNED_IP_TO_CLIENT,
                &WIFIEventHandler,
                NULL,
                NULL
            )
        );
    }
};
