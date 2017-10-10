#include "upreport.h"

#include <gtest/gtest.h>
#include <pthread.h>
#include <common_function.h>
#include <string>
#include "log.h"
#include "defines.h"

using namespace std;
using namespace cems::service::scan;

#define SERVER_IP "172.16.18.41"
#define CENTER_PORT 9000
#define THREAD_NUM 20
#define SEND_NUM 50
#define TRANS_NUM 5
#define LOG_CONFIG_PATH "../config/log4cplus.properties"

int main(int args, char* argv[])
{
    INIT_LOG(LOG_CONFIG_PATH);
    testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}

void* thread_fun(void* arg)
{
    UpReport* context = (UpReport*)arg;
    std::string szText = "[{areaId:serverAreaMain}]";
    for(int i = 0; i < SEND_NUM; i++)
    {
        bool bret = context->SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
        if(!bret)
        {
            LOG_ERROR("SendToServer: send data to center service error, data:"<<szText.c_str());
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

TEST(test_transport_pool, case1)
{
    UpReport datacenter_report;
    datacenter_report.Init(SERVER_IP, CENTER_PORT, TRANS_NUM);
    int i = 0;
    do
    {
        if(datacenter_report.Open() == false)
        {
            LOG_ERROR("data center report init error.");
            datacenter_report.Close();
            i++;
            sleep(6);
            continue;
        }

        pthread_t threads[THREAD_NUM];
        for(int i = 0; i < THREAD_NUM; i++)
        {
            int res = pthread_create(&(threads[i]), NULL, thread_fun, &datacenter_report);
            if(res < 0)
            {
                LOG_ERROR("create thread "<<i<<" error.");
                break;
            }
        }

        for(int i = 0; i < THREAD_NUM; i++)
        {
            int res = pthread_join(threads[i], NULL);
            if(res < 0)
            {
                LOG_ERROR("join thread "<<i<<" error.");
                break;
            }
        }
        datacenter_report.Close();
        i++;
    }while(i < 3);
}


