#include "log.h"

#define LOG_CONFIG_PATH "./log4cplus.properties"

void fun()
{
    LOG_INFO("zhege shi shenme");
}

int main()
{
    INIT_LOG(LOG_CONFIG_PATH);
// SetSignal();
//
 
    fun();
    int x = 5;

    LOG_TRACE("trace" << x);
    LOG_DEBUG("debug");
    LOG_ERROR("error" << x);
    LOG_INFO("info");
    LOG_FATAL("fatal");
    return 0;
}
