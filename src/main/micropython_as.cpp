// main include headers
#include "ClientHandler.hpp"
#include "NetworkWifi.hpp"
#include "Micropython.hpp"

// include constants
#include "constants.h"

extern "C" void app_main(void)
{
  uint16_t fd = 1;
  ClientHandler testhandler;
  testhandler.addClient(fd);

  NetworkWifi net;
  net.addClient();

  Micropython interpreter;
  interpreter.test();
}
