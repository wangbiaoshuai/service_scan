#ifndef _TRANSPORT_POOL_H
#define _TRANSPORT_POOL_H

#include "CommonService.h"
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TBinaryProtocol.h>
#include <vector>
#include <string>
#include <pthread.h>

using namespace boost;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace com::vrv::cems::common::thrift::service;
using boost::shared_ptr;

class Transport
{
public:
	boost::shared_ptr<TSocket>     m_socket; 
	boost::shared_ptr<TTransport>  m_transport;
	boost::shared_ptr<TProtocol>   m_protocol;	
    boost::shared_ptr<CommonServiceClient> m_pclient;
    volatile int m_use;  //0没有被使用，-1不可用
    Transport(): m_use(1) {}
};

class TransportPool
{
public:
    TransportPool();
    ~TransportPool();
    int Init(const std::string& ip, int port, int size = 0);
    Transport* GetTransport();
    int FreeTransport(Transport* trans);
    int DeleteTransport(Transport* trans);
    void Destroy();
private:
    std::vector<Transport> transport_pool_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;

private:
    volatile int max_size_;   //最大连接数
    volatile int min_size_;   //最小连接数
    volatile int cur_size_;   //当前连接数
    //int max_idle_time_;   //空闲时间，超过该时间，删除连接数到最低
    volatile int idle_num_;   //空闲的连接数
};
#endif // _TRANSPORT_POOL_H
