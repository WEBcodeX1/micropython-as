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

    static esp_netif_ip_info_t IPAddrStruct;

    static string IPAddr;
    static string IPGateway;
    static string IPNetmask;

public:

    static bool StaticIP;

    static void init()
    {
        esp_netif_init();
        esp_event_loop_create_default();
    }

    static void activateIPAddr(esp_netif_t* NetInterface)
    {
        memset(&IPAddrStruct, 0, sizeof(esp_netif_ip_info_t));
        IPAddrStruct.ip.addr = esp_ip4addr_aton(IPAddr.c_str());
        IPAddrStruct.gw.addr = esp_ip4addr_aton(IPGateway.c_str());
        IPAddrStruct.netmask.addr = esp_ip4addr_aton(IPNetmask.c_str());
        esp_netif_set_ip_info(NetInterface, &IPAddrStruct);
    }

    static void reconfigureDHCP(esp_netif_t* NetInterface)
    {
        //- stop dhcp server
        esp_netif_dhcps_stop(NetInterface);

        //- activate ip address
        activateIPAddr(NetInterface);

        // 4. Start DHCP server with the new settings
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
