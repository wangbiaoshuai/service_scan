#include "transport_pool.h"
#include "log.h"

using namespace std;

TransportPool::TransportPool():
transport_pool_(),
max_size_(50),
min_size_(5),
cur_size_(0),
//max_idle_time_(5),
idle_num_(0)
{
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);
}

TransportPool::~TransportPool()
{
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
}

int TransportPool::Init(const std::string& ip, int port, int size)
{
    if(ip.empty() || port <= 0 || size > max_size_)
    {
        LOG_ERROR("TransportPool::Init failed, ip/port is illegal.");
        return -1;
    }
    if(size == 0)
        size = min_size_;

    for(register int i = 0; i < size; i++)
    {
        Transport transport;
        transport.m_socket = boost::shared_ptr<TSocket>(new TSocket(ip.c_str(), port));
        transport.m_socket->setConnTimeout(1000 * 5); // set connection timeout 5S
        transport.m_socket->setRecvTimeout(1000 * 30);
        transport.m_socket->setSendTimeout(1000 * 30);
        transport.m_transport = boost::shared_ptr<TTransport>(new TFramedTransport(transport.m_socket));
        transport.m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(transport.m_transport));
        transport.m_pclient = boost::shared_ptr<CommonServiceClient>(new CommonServiceClient(transport.m_protocol));
        transport.m_use = 1;
        try
        {
            transport.m_transport->open();
        }
        catch(TTransportException te)
        {
            string exception(te.what());
            LOG_ERROR("Init: catch TTransportException("<<exception.c_str()<<").");
            return -1;
        }
        catch(TException tx)
        {
            LOG_ERROR("Init: catch an TException.");
            return -1;
        }
        catch(...)
        {
            LOG_ERROR("Init: catch an Exception.");
            return -1;
        }

        pthread_mutex_lock(&mutex_);
        transport_pool_.push_back(transport);
        cur_size_++;
        idle_num_++;
        pthread_mutex_unlock(&mutex_);
    }
    LOG_INFO("Init: "<<cur_size_<<" trans has been opened, "<<idle_num_<<" trans is idle.");
    return 0;
}

Transport* TransportPool::GetTransport()
{
    Transport* ret = NULL;
    pthread_mutex_lock(&mutex_);
    if(cur_size_ == 0)
    {
        LOG_WARN("GetTransport: get failed, pool size is "<<cur_size_<<", idle num is "<<idle_num_<<".");
        pthread_mutex_unlock(&mutex_);
        return ret;
    }
    while(cur_size_ != 0 && idle_num_ == 0)  // 防止惊群效应
    {
        struct timeval now;
        struct timespec wait_time;
        gettimeofday(&now, NULL);
        wait_time.tv_sec = now.tv_sec + 5;
        wait_time.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&cond_, &mutex_, &wait_time);
    }

    vector<Transport>::iterator it;
    for(it = transport_pool_.begin(); it != transport_pool_.end(); ++it)
    {
        if(it->m_use == 1)
        {
            ret = &(*it);
            it->m_use = 0;
            idle_num_ --;
            LOG_DEBUG("GetTransport: get one trans, remainder "<<idle_num_<<" trans.");
            break;
        }
    }
    pthread_mutex_unlock(&mutex_);
    return ret;
}

int TransportPool::FreeTransport(Transport* trans)
{
    if(trans == NULL)
    {
        LOG_ERROR("FreeTransport: trans is NULL.");
        return -1;
    }

    pthread_mutex_lock(&mutex_);
    trans->m_use = 1;
    idle_num_ ++;
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
    trans = NULL;
    LOG_DEBUG("FreeTransport: give back one trans, remainder "<<idle_num_<<" trans.");
    return 0;
}

#if 0
int TransportPool::DeleteTransport(Transport* trans)
{
    if(trans == NULL)
    {
        LOG_ERROR("DeleteTransport: trans is NULL.");
        return -1;
    }

    pthread_mutex_lock(&mutex_);
    vector<Transport>::iterator it;
    for(it = transport_pool_.begin(); it != transport_pool_.end(); ++it)
    {
        if(&(*it) == trans)
            break;
    }

    if(it != transport_pool_.end())
    {
        transport_pool_.erase(it);
        //idle_num_ --;
        cur_size_ --;
        LOG_WARN("DeleteTransport: "<<trans<<" trans loss effective, deleted it, pool size is "<<cur_size_<<",remainder "<<idle_num_<<" trans.");
    }
    pthread_mutex_unlock(&mutex_);
    pthread_cond_broadcast(&cond_);
    return 0;
}
#endif
int TransportPool::DeleteTransport(Transport* trans)
{
    if(trans == NULL)
    {
        LOG_ERROR("DeleteTransport: trans is NULL.");
        return -1;
    }

    pthread_mutex_lock(&mutex_);
    idle_num_++;
    trans->m_use = -1;
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
    LOG_WARN("DeleteTransport: "<<trans<<" trans loss effective, pool size is "<<cur_size_<<", remainder "<<idle_num_<<" trans.");
    return 0;
}

void TransportPool::Destroy()
{
    pthread_cond_broadcast(&cond_);
    pthread_mutex_lock(&mutex_);
    vector<Transport>::iterator it;
    for(it = transport_pool_.begin(); it != transport_pool_.end(); ++it)
    {
        try
        {
            it->m_transport->close();
        }
        catch(TTransportException te)
        {
            string exception(te.what());
            LOG_ERROR("Destroy: close error("<<exception.c_str()<<").");
        }
        catch(TException tx)
        {
            string exception(tx.what());
            LOG_ERROR("Destroy: close error("<<exception.c_str()<<").");
        }
    }
    
    vector<Transport> ().swap(transport_pool_);  //1.清空vector  2.释放内存
    cur_size_ = 0;
    idle_num_ = 0;
    pthread_mutex_unlock(&mutex_);

    LOG_INFO("Destroy: transport pool is destoryed.");
    return;
}
