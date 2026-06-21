#pragma once

#include "esp_wifi.h"
#include "esp_log.h"

#include <cstring>

#include "constants.h"

#include "Network.hpp"


// - external RTOS C code
extern "C" {
    #include "freertos/event_groups.h"

    static EventGroupHandle_t WifiEventHandle;

    static void WIFIEventHandler(
        void* Arguments,
        esp_event_base_t EventBase,
        int32_t EventId,
        void* EventData
    ){
        if (EventBase == WIFI_EVENT && EventId == WIFI_EVENT_AP_STACONNECTED) {
            xEventGroupSetBits(WifiEventHandle, WIFI_CONNECTED_BIT);
        } else if (EventBase == WIFI_EVENT && EventId == WIFI_EVENT_AP_STADISCONNECTED) {
            xEventGroupSetBits(WifiEventHandle, WIFI_DISCONNECTED_BIT);
        } else if (EventBase == IP_EVENT && EventId == IP_EVENT_ASSIGNED_IP_TO_CLIENT) {
            xEventGroupSetBits(WifiEventHandle, WIFI_STA_IP_ASSIGNED_BIT);
        }
    }

    static EventGroupHandle_t getWifiEventHandle() {
        return WifiEventHandle;
    }
}


class NetworkWifi
{

private:

public:

    static void initDefaultConfig()
    {
        wifi_init_config_t APDefaultConfig = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&APDefaultConfig));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    }

    static void initSoftAP()
    {
        esp_netif_t* NetInterface = esp_netif_create_default_wifi_ap();

        wifi_config_t APConfig;

        memcpy(&APConfig.ap.ssid, &WIFI_AP_SSID[0], strlen(WIFI_AP_SSID)+1);
        APConfig.ap.ssid_len = strlen(WIFI_AP_SSID);
        APConfig.ap.channel = WIFI_CHANNEL;
        APConfig.ap.max_connection = WIFI_MAX_STA_CONN;
        APConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
        APConfig.ap.pmf_cfg.required = false;

        if (strlen(WIFI_AP_PASSWD) == 0) {
            APConfig.ap.authmode = WIFI_AUTH_OPEN;
        }
        else {
            memcpy(&APConfig.ap.password, &WIFI_AP_PASSWD[0], strlen(WIFI_AP_PASSWD)+1);
        }

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &APConfig));

        ESP_LOGI("WifiInit", "Init finished. SSID:%s password:%s channel:%d", APConfig.ap.ssid, APConfig.ap.password, APConfig.ap.channel);

        ESP_ERROR_CHECK(esp_wifi_start());

        if (Network::StaticIP == true) {
            Network::reconfigureDHCP(NetInterface);
        }
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
