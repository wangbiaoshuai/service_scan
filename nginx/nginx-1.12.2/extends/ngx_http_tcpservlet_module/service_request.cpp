#include "service_request.h"

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TBinaryProtocol.h>
#include <sstream>

#include "json/json.h"
#include "common.h"
#include "CacheService.h"

#ifdef _DEBUG
#include <stdio.h>
#endif

using namespace boost;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace com::vrv::cems::common::thrift::service;
using namespace com::vrv::cems::service::base::interfaces;
using namespace std;

ServiceRequest::ServiceRequest():
server_ip_(""),
server_port_(""),
session_map_lock_(),
session_map_(),
service_addr_map_lock_(),
service_addr_map_(),
log_(NULL)
{
    printf("ServiceRequest constructor.\n");
}

ServiceRequest::~ServiceRequest()
{
}

int ServiceRequest::SetAddr(const std::string& ip, const std::string& port)
{
    printf("SetAddr: ip=%s, port=%s\n", ip.c_str(), port.c_str());
    if(ip.empty() || port.empty())
    {
        ngx_log_error(NGX_LOG_ERR, log_, 0, "SetAddr: ip or port is empty.");
        return -1;
    }
    server_ip_ = ip;
    server_port_ = port;
    return 0;
}

void ServiceRequest::QueryService(std::vector<ServiceConfigBean>& service_info, const std::string& service_code, const std::string& version)
{
    if(server_ip_.empty() || server_port_.empty())
    {
        ngx_log_error(NGX_LOG_ERR, log_, 0, "queryService: service ip or port is empty.");
        return;
    }

    try
    {
        boost::shared_ptr<TSocket> socket(new TSocket(server_ip_.c_str(), atoll(server_port_.c_str())));
        socket->setConnTimeout(1000 * RPC_CONN_TIMEOUT); // set connection timeout 5S
        socket->setRecvTimeout(1000 * RPC_RECV_TIMEOUT);
        boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        ConfigServiceClient client(protocol);

        transport->open();
        client.queryService(service_info, service_code, version);
        transport->close();
    }
    catch(TException & tx)
    {
        string exception(tx.what());
        ngx_log_error(NGX_LOG_ERR, log_, 0, "QueryService: thrift exception: %s", exception.c_str());
    }
    return;
}

bool ServiceRequest::Fetch(const std::string& service_code, std::string& ip, std::string& port)
{
    if(service_code.empty())
    {
        ngx_log_error(NGX_LOG_ERR, log_, 0, "ServiceRequest::Fetch service code is empty.");
        return false;
    }

    vector<ServiceConfigBean> service_info;
    std::string version = "1.0"; //版本号必须是1.0，否则查询不到服务

    QueryService(service_info, service_code, version);

    if(service_info.empty())
    {
        ngx_log_error(NGX_LOG_ERR, log_, 0, "ServiceRequest::Fetch can not query service %s info.", service_code.c_str());
        return false;
    }

    bool fetch = false;

    for(register size_t i = 0; i < service_info.size(); i++)
    {
        ServiceConfigBean bean = service_info.at(i);
        if(service_code == bean.serviceID)
        {
            ip = bean.ip;

            stringstream ss;
            ss << bean.port;
            port = ss.str();
            fetch = true;
            break;
        }
    }

    return fetch;
}

string ServiceRequest::GetdataTC(const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& jdata, const string& session_id, const int32_t msg_code)
{
    string ip, port;
    int find = 0;
    service_addr_map_lock_.Lock();
    if(service_addr_map_.find(maxCode) != service_addr_map_.end())
    {
        ip = service_addr_map_[maxCode].ip_;
        port = service_addr_map_[maxCode].port_;
        find = 1;
    }
    service_addr_map_lock_.Unlock();

    if(!find)
    {
        if(!Fetch(maxCode, ip, port) || ip.empty() || port.empty())
        {
            return "";
        }
        service_addr_map_lock_.Lock();
        Address addr;
        addr.ip_ = ip;
        addr.port_ = port;
        service_addr_map_[maxCode] = addr;
        service_addr_map_lock_.Unlock();
    }
    string res("");
    try
    {
        boost::shared_ptr<TSocket> socket(new TSocket(ip, atoll(port.c_str())));
        socket->setConnTimeout(1000 * RPC_CONN_TIMEOUT); // set connection timeout 5S
        socket->setRecvTimeout(1000 * RPC_RECV_TIMEOUT);
        boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        CommonServiceClient client(protocol);

        transport->open();
        client.getDataTC(res, maxCode, minCode, checkCode, isZip, jdata, session_id, msg_code);
        transport->close();
    }
    catch(TException & tx)
    {
        string exception(tx.what());
        service_addr_map_lock_.Lock();
        service_addr_map_.erase(maxCode);
        service_addr_map_lock_.Unlock();
        ngx_log_error(NGX_LOG_ERR, log_, 0, "RequestService: thrift exception: %s", exception.c_str());
    }
    return res;
}

string int2stringhex(unsigned int data)
{
    char buf[9] = {0};
    sprintf(buf, "%08X", data);
    return string(buf, strlen(buf));
}

string session2stringhex(const unsigned char* data, int len)
{
    string result;
    char buf[3] = {0};
    data = data + len - 1;
    for(int i = 0; i < len; i++, data--)
    {
        sprintf(buf, "%02X", *data);
        result = result + string(buf, strlen(buf));
    }
    return result;
}

int ServiceRequest::ParseMsgBody(const void* msg, unsigned int len, PARAM_INFO& param_info)
{
    if(msg == NULL || len <= sizeof(CEMS_NET_HEAD))
    {
        ngx_log_error(NGX_LOG_ERR, log_, 0, "ServiceRequest::ParseMsgBody msg error, len=%d", len);
        return -1;
    }

    char* ptr = (char*)msg;
    PCEMS_NET_HEAD p_header = (PCEMS_NET_HEAD)msg;
    param_info.maxcode_ = int2stringhex(p_header->dwMaxCode);
    param_info.mincode_ = int2stringhex(p_header->dwMinCode);
    param_info.checkcode_ = int2stringhex(p_header->dwCrc);
    param_info.msgcode_ = p_header->dwMsgCode;
    param_info.is_zip_ = (bool)(p_header->wType);
    ptr += sizeof(CEMS_NET_HEAD);
    param_info.jdata_ = string(ptr, 0, len - sizeof(CEMS_NET_HEAD));
    param_info.session_id_ = session2stringhex(p_header->szSessionId, 16);
    /*DeviceKeyCache device_key;
    if(QueryDeviceKeyByCache(param_info.session_id_, device_key) < 0 || device_key.keyType.empty() || device_key.password.empty())
    {
        printf("QueryDeviceKeyByCache failed.\n");
        return -1;
    }
    param_info.is_encrypt_ = true;
    param_info.key_ = device_key.password;
    param_info.flag_ = atoi(device_key.keyType.c_str());*/
#ifdef _DEBUG
    ngx_log_error(NGX_LOG_DEBUG, log_, 0, "maxcode: %s, mincode: %s, checkcode: %s, jdata: %s, is_zip: %d, session_id: %s\n", param_info.maxcode_.c_str(), param_info.mincode_.c_str(), param_info.checkcode_.c_str(), param_info.jdata_.c_str(), p_header->wType, param_info.session_id_.c_str());
    //printf("maxcode: %s, mincode: %s, checkcode: %s, jdata: %s, is_zip: %d, session_id: %s\n", param_info.maxcode_.c_str(), param_info.mincode_.c_str(), param_info.checkcode_.c_str(), param_info.jdata_.c_str(), p_header->wType, param_info.session_id_.c_str());
#endif
    return 0;
}

string ServiceRequest::GetdataTC(const PARAM_INFO& param_info)
{
    string result("");
    result = GetdataTC(param_info.maxcode_, param_info.mincode_, param_info.checkcode_, param_info.is_zip_, param_info.jdata_, param_info.session_id_, param_info.msgcode_);
    return result;
}

int ServiceRequest::QueryDeviceKeyByCache(const std::string session_id, DeviceKeyCache& device)
{
    map<string, DeviceKeyCache>::iterator it;
    session_map_lock_.Lock();
    if(session_map_.find(session_id) != session_map_.end())
    {
        device = session_map_[session_id];
        session_map_lock_.Unlock();
        return 0;
    }
    session_map_lock_.Unlock();

    string ip, port;
    if(!Fetch(SERVICE_CACHE_MAXCODE, ip, port) || ip.empty() || port.empty())
    {
        return -1;
    }

    try
    {
        boost::shared_ptr<TSocket> socket(new TSocket(ip, atoll(port.c_str())));
        socket->setConnTimeout(1000 * RPC_CONN_TIMEOUT); // set connection timeout 5S
        socket->setRecvTimeout(1000 * RPC_RECV_TIMEOUT);
        boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        CacheServiceClient client(protocol);

        transport->open();
        client.queryDeviceKeyBySessionId(device, SERVICE_CACHE_MAXCODE, "305", session_id);
        transport->close();
    }
    catch(TException & tx)
    {
        string exception(tx.what());
        ngx_log_error(NGX_LOG_ERR, log_, 0, "QueryDeviceKeyByCache: thrift exception: %s", exception.c_str());
        return -1;
    }

    session_map_lock_.Lock();
    if(session_map_.size() >= MAP_MAX_SIZE)
    {
        map<string, DeviceKeyCache> ().swap(session_map_);
    }
    session_map_[session_id] = device;
    session_map_lock_.Unlock();
    return 0;
}

void ServiceRequest::SetLog(ngx_log_t* log)
{
    log_ = log;
}
