#pragma once

#include "esp_netif.h"
#include "esp_event.h"
#include "fcntl.h"

#include <cstring>
#include <string>


using namespace std;


class Network
{

private:

    static string IPAddr;
    static string IPGateway;
    static string IPNetmask;

public:

    static bool StaticIP;

    static void init()
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
    }

    static void activateIPAddr(esp_netif_t* NetInterface)
    {
        esp_netif_ip_info_t IPAddrStruct;
        memset(&IPAddrStruct, 0, sizeof(esp_netif_ip_info_t));
        IPAddrStruct.ip.addr = esp_ip4addr_aton(IPAddr.c_str());
        IPAddrStruct.gw.addr = esp_ip4addr_aton(IPGateway.c_str());
        IPAddrStruct.netmask.addr = esp_ip4addr_aton(IPNetmask.c_str());
        esp_netif_set_ip_info(NetInterface, &IPAddrStruct);
    }

    static void reconfigureDHCP(esp_netif_t* NetInterface)
    {
        //- stop DHCP server
        esp_netif_dhcps_stop(NetInterface);

        //- activate ip address
        activateIPAddr(NetInterface);

        // re-start DHCP server
        esp_netif_dhcps_start(NetInterface);
    }
};


class Socket {

public:

    static void makeNonblocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK );
    }

};
