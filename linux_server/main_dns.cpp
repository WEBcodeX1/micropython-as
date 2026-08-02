#include "DNSServer.hpp"
#include "Network.hpp"

#include <csignal>
#include <cstdlib>
#include <limits>
#include <string>

using namespace std;

bool   Network::StaticIP   = false;
string Network::IPAddr     = "127.0.0.1";
string Network::IPGateway  = "";
string Network::IPNetmask  = "";

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIG_IGN);

    if (argc >= 2) {
        Network::setIPAddr(string(argv[1]));
    }

    uint16_t ListenPort = 53535;
    if (argc >= 3) {
        char* EndPointer = nullptr;
        const unsigned long ParsedPort = strtoul(argv[2], &EndPointer, 10);

        if (EndPointer == argv[2] || *EndPointer != '\0' || ParsedPort > numeric_limits<uint16_t>::max()) {
            return 1;
        }

        ListenPort = static_cast<uint16_t>(ParsedPort);
    }

    DNSServer DNSServerRef(ListenPort);
    DNSServerRef.start();

    return 0;
}
