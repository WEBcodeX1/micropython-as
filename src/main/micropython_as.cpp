// main include headers
#include "Network.hpp"
#include "NetworkWifi.hpp"
#include "Micropython.hpp"
#include "ClientHandler.hpp"

// include constants
#include "constants.h"

//- set network config
bool Network::StaticIP = true;
std::string IPAddr = "192.168.10.254";
std::string IPGateway = "192.168.10.254";
std::string IPNetmask = "255.255.255.0";

//- main loop
extern "C" void app_main(void)
{
  uint16_t fd = 1;
  ClientHandler testhandler;
  testhandler.addClient(fd);

  MicroPython interpreter;

  string ResultString;
  string FunctionName("testfunc");
  string FunctionParam("testparam");

  const bool status = interpreter.callFunction(
    FunctionName, FunctionParam, ResultString
  );
}
