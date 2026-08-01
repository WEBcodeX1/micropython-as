// Linux server entry point for stability testing of the HTTP server component.
//
// Usage:
//   ./server_linux [listen-address]
//
//   listen-address  IPv4 address the server binds to (default: 0.0.0.0)
//
// The server listens on port 8080.  Run 'curl http://localhost:8080/' to exercise it.
// Build with AddressSanitizer or Valgrind for detailed crash analysis.

#include "./filesystem/Filesystem.hpp"

#include "Server.hpp"
#include "Network.hpp"
#include "ASRequestHandler.hpp"
#include "ASRequestDef.hpp"
#include "ASRequestGlobal.hpp"

#include <signal.h>
#include <string>
#include <thread>

using namespace std;

// ---------------------------------------------------------------------------
// Global definitions expected by the server / request handler subsystem
// ---------------------------------------------------------------------------

// Network static members
bool   Network::StaticIP   = false;
string Network::IPAddr     = "0.0.0.0";
string Network::IPGateway  = "";
string Network::IPNetmask  = "";

// AS request exchange globals (shared between server and game logic)
unsigned int ASRequestStatus        = AS_REQ_WAIT_IN;
unsigned int ASRequestID            = 0;
unsigned int ASRequestContentLength = 0;
char         ASRequestExchangeBuffer[2048];

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // Ignore SIGPIPE so that write() to a closed socket returns EPIPE instead
    // of killing the process (triggered by test_client --test midclose).
    signal(SIGPIPE, SIG_IGN);

    if (argc >= 2) {
        Network::setIPAddr(string(argv[1]));
    }

    Server ServerRef;
    ServerRef.start();

    return 0;
}
