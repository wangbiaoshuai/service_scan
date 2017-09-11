#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "transferserver.h"
#include "./log/util_log.h"

int main(int argc, char* argv[])
{
  // try
  // {
    if (argc != 3)
    {
      std::cerr << "Usage: async_tcp_echo_server <port1> <port2>\n";
      return 1;
    }

    //start log for all kinds info
    log_start();
    log_SetType(LOG_TYPE_ALL);
    log_SetLogName("../logs/Sys.log");

    boost::asio::io_service io_service;

    using namespace std; // For atoi.
    transfer::server mousekey_server(io_service, atoi(argv[1]));
    transfer::server screen_server(io_service, atoi(argv[2]));

    io_service.run();
  // }
  // catch (std::exception& e)
  // {
  //   std::cerr << "Exception: " << e.what() << "\n";
  // }

  return 0;
}
