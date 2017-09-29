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
}

TransportPool::~TransportPool()
{
}

int TransportPool::Init(const std::string& ip, int port, int size)
{
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);

    if(ip.empty() || port <= 0 || size > max_size_)
    {
        return -1;
    }
    if(size == 0)
        size = min_size_;

    for(register int i = 0; i < size; i++)
    {
        Transport transport;
        transport.m_socket = boost::shared_ptr<TSocket>(new TSocket(ip.c_str(), port));
        transport.m_socket->setConnTimeout(1000 * 5); // set connection timeout 5S
        transport.m_transport = boost::shared_ptr<TTransport>(new TFramedTransport(transport.m_socket));
        transport.m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(transport.m_transport));
        transport.m_pclient = boost::shared_ptr<CommonServiceClient>(new CommonServiceClient(transport.m_protocol));
        transport.m_use = false;
        try
        {
            transport.m_transport->open();
        }
        catch(TException & tx)
        {
            LOG_ERROR("Init: open connect ip=" << ip.c_str() << ", port=" << port << "open-ERROR:" << tx.what());

            return -1;
        }

        pthread_mutex_lock(&mutex_);
        transport_pool_.push_back(transport);
        cur_size_++;
        idle_num_++;
        pthread_mutex_unlock(&mutex_);
    }
    LOG_INFO("Init: "<<size<<" trans has been opened, "<<idle_num_<<" trans is idle.");
    return 0;
}

Transport* TransportPool::GetTransport()
{
    Transport* ret = NULL;
    pthread_mutex_lock(&mutex_);
    while(idle_num_ == 0)  // 防止惊群效应
    {
        pthread_cond_wait(&cond_, &mutex_);
    }

    vector<Transport>::iterator it;
    for(it = transport_pool_.begin(); it != transport_pool_.end(); ++it)
    {
        if(it->m_use == false)
        {
            ret = &(*it);
            it->m_use = true;
            idle_num_ --;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_);
    LOG_DEBUG("GetTransport: get one trans, remainder "<<idle_num_<<" trans.");
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
    trans->m_use = false;
    idle_num_ ++;
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
    trans = NULL;
    LOG_DEBUG("FreeTransport: give back one trans, remainder "<<idle_num_<<" trans.");
    return 0;
}

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
        idle_num_ --;
        cur_size_ --;
        LOG_WARN("DeleteTransport: "<<trans<<" trans loss effective, remainder "<<idle_num_<<"trans.");
    }
    pthread_mutex_unlock(&mutex_);
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
        catch(TException & tx)
        {
            LOG_ERROR("Destroy: close error("<<tx.what()<<")");
        }
    }
    
    transport_pool_.clear();
    cur_size_ = 0;
    idle_num_ = 0;
    pthread_mutex_unlock(&mutex_);

    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
    LOG_INFO("Destroy: transport pool is destoryed.");
    return;
}
