#include "service_request.h"

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TBinaryProtocol.h>
#include <sstream>

#include "json/json.h"

using namespace boost;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace com::vrv::cems::common::thrift::service;
using namespace std;

ServiceRequest::ServiceRequest(const string& ip, const string& port):
server_ip_(ip),
server_port_(port)
{
}

ServiceRequest::~ServiceRequest()
{
}

void ServiceRequest::QueryService(std::vector<ServiceConfigBean>& service_info, const std::string& service_code, const std::string& version)
{
    //LOG_INFO("Begin query service:" << service_code << " info.");
    if(server_ip_.empty() || server_port_.empty())
    {
        //LOG_ERROR("QueryService: server ip or port is empty");
        return;
    }

    boost::shared_ptr<TSocket> socket(new TSocket(server_ip_.c_str(), atoll(server_port_.c_str())));
    socket->setConnTimeout(1000 * 5); // set connection timeout 5S
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ConfigServiceClient client(protocol);

    try
    {
        transport->open();
        client.queryService(service_info, service_code, version);
        transport->close();
    }
    catch(TException & tx)
    {
        string exception(tx.what());
        //LOG_ERROR("QueryService: thrift exception:" << exception.c_str());
    }
    return;
}

bool ServiceRequest::Fetch(const std::string& service_code, std::string& ip, std::string& port)
{
    if(service_code.empty())
    {
        //LOG_ERROR("Fetch: service code is empty");
        return false;
    }

    vector<ServiceConfigBean> service_info;
    std::string version = "1.0"; //版本号必须是1.0，否则查询不到服务

    QueryService(service_info, service_code, version);

    if(service_info.empty())
    {
        //LOG_ERROR("Fetch: can not query service:" << service_code << " info.");
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

string ServiceRequest::GetdataTS(const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& jdata, const bool isEncrypt, const std::string& key, const int32_t flag)
{
    string ip, port;
    if(!Fetch(maxCode, ip, port) || ip.empty() || port.empty())
    {
        return "";
    }
    string res;
    boost::shared_ptr<TSocket> socket(new TSocket(ip, atoll(port.c_str())));
    socket->setConnTimeout(1000 * 5); // set connection timeout 5S
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    CommonServiceClient client(protocol);
    try
    {
        transport->open();
        client.getDataTS(res, maxCode, minCode, checkCode, isZip, jdata, isEncrypt, key, flag);
        transport->close();
    }
    catch(TException & tx)
    {
        string exception(tx.what());
        //LOG_ERROR("RequestService: thrift exception:" << exception.c_str());
    }
    return res;
}
