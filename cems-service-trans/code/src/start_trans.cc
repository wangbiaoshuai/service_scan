#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <pthread.h>
#include "transferserver.h"
#include "log.h"

#define MOUSE_PORT 9010
#define SCREEN_PORT 9011

/*int main(int argc, char* argv[])
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
}*/


pthread_t trans_thread;

void* thread_function(void* context)
{
    try
    {
        LOG_INFO("transfer thread start.");
        boost::asio::io_service io_service;

        using namespace std; // For atoi.
        transfer::server mousekey_server(io_service, MOUSE_PORT);
        transfer::server screen_server(io_service, SCREEN_PORT);

        io_service.run();
    }
    catch(std::exception& e)
    {
        LOG_ERROR("thread_function: catch exception("<<e.what()<<").");
    }
    catch(...)
    {
        LOG_ERROR("thread_function: catch an exception.");
    }
    LOG_INFO("transfer thread end.");
    pthread_exit(NULL);
}

int StartTrans()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    
    int res = pthread_create(&trans_thread, &attr, thread_function, NULL);
    if(res != 0)
    {
        LOG_ERROR("StartTrans: create thread error: "<<strerror(errno));
    }
    pthread_attr_destroy(&attr);
    return res;
}
