#include "service_reg.h"

#include <log.h>
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TBinaryProtocol.h>

#include "parse_configure.h"
#include "common_function.h"
#include "defines.h"
#include "gen_algorithm.h"
#include "json/json.h"
#include "CommonService.h"
#include "md5module.h"

namespace cems{ namespace service{ namespace scan{

using namespace boost;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace com::vrv::cems::common::thrift::service;
using namespace std;

ServiceReg::ServiceReg():
service_bean_(),
server_ip_(""),
server_port_(""),
server_areaID_(""),
heart_id_(""),
heart_status_(true),
heart_thread_()
{
    ParseConfigure::GetInstance().Init();
    ReadConfig();
}

bool ServiceReg::ReadConfig()
{
    string key("server.ip");
    ParseConfigure::GetInstance().GetProperty(key, server_ip_);

    key = "server.port";
    ParseConfigure::GetInstance().GetProperty(key, server_port_);

    key = "service.serverAreaId";
    ParseConfigure::GetInstance().GetProperty(key, server_areaID_);
    if(server_ip_.empty() || server_port_.empty())
    {
        LOG_ERROR("ReadConf: get server ip or port failed");
        return false;
    }

    key = "service.ip";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.ip);

    key = "service.port";
    string value;
    ParseConfigure::GetInstance().GetProperty(key, value);
    service_bean_.port = stoul(value);

    key = "service.code";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.serviceID);

    key = "service.version";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.version);

    service_bean_.orgID = server_areaID_;
    /*key = "service.serverAreaId";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.orgID);*/

    service_bean_.SSID = GenericSSID();
    service_bean_.serverID  = GenericUUID(service_bean_.ip);

    key = "service.name";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.name);
    
    service_bean_.location = GetCurrentPath();

    key = "service.description";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.description);

    key = "service.installTime";
    ParseConfigure::GetInstance().GetProperty(key, value);
    if(!value.empty())
    {
        service_bean_.installTime = stoll(value);
    }

    LOG_INFO("ServiceReg::ReadConfig success!");
    return true;
}

ServiceReg::~ServiceReg()
{
}

void ServiceReg::WriteConfig()
{
    string key = "service.heartId";
    ParseConfigure::GetInstance().SetProperty(key, heart_id_, 0);

    key = "service.serverAreaId";
    ParseConfigure::GetInstance().SetProperty(key, service_bean_.orgID, 0);

    key = "service.id";
    ParseConfigure::GetInstance().SetProperty(key, service_bean_.SSID, 0);

    key = "service.installTime";
    time_t time = service_bean_.installTime;
    char tmBuf[100] = {0};
    strftime(tmBuf, 100, "%Y-%m-%d %H:%M:%S", localtime(&time));
    stringstream ss;
    ss << tmBuf;
    ParseConfigure::GetInstance().SetProperty(key, ss.str(), 0);

    key = "service.location";
    ParseConfigure::GetInstance().SetProperty(key, service_bean_.location, 0);

    ParseConfigure::GetInstance().Flush();
}

int32_t ServiceReg::GetServicePort()
{
    return service_bean_.port;
}

bool ServiceReg::IsLegalAddr()
{
    vector<string> ips;
    GetIps(ips);
    if(ips.empty())
    {
        LOG_WARN("IsLegalAddr: GetIps failed");
        return true;
    }

    register size_t i;
    for(i = 0; i < ips.size(); i++)
    {
        if(service_bean_.ip == ips[i])
        {
            LOG_INFO("service ip:" << service_bean_.ip);
            return true;
        }
    }
    if(i == ips.size())
    {
        return false;
    }
    return true;
}

std::string ServiceReg::GetAreaId()
{
    string res;
    std::string max_code = SERVICE_CODE_ADDRESS; //"00FF0600";
    std::string min_code = MINCODE_ADDRESS_AREAID;      //"8";

    Json::Value root, child;
    child["serverId"] = service_bean_.serverID;
    root.append(child);
    Json::FastWriter writer;
    string jdata;
    jdata = writer.write(root);

    string addr_ip, addr_port;
    if(GetServiceIp(max_code, "", addr_ip, addr_port) == false)
    {
        LOG_ERROR("GetAreaId: GetServiceIp failed");
        return res;
    }

    res = RequestService(addr_ip, addr_port, max_code, min_code, false, jdata);
    string areaID;
    if(!res.empty())
    {
        Json::Value root;
        Json::Reader reader;
        if(reader.parse(res, root))
        {
            if(root["result"] != "0")
            {
                LOG_ERROR("GetAreaId: RequestService result=0");
                return "";
            }
            if(!root["jdata"].isNull())
            {
                Json::Value jdata = root["jdata"];
                for(register size_t i = 0; i < jdata.size(); i++)
                {
                    Json::Value temp = jdata[i];
                    areaID = temp["serverAreaId"].isNull() ? "" : temp["serverAreaId"].asString();
                }
            }
        }
    }
    return areaID;
}

bool ServiceReg::RegistToConfSrv()
{
    LOG_INFO("begin RegistToConfSrv");
    bool ret = true;

    service_bean_.orgID = GetAreaId();
    if(service_bean_.orgID.empty())
    {
        LOG_ERROR("RegistToConfSrv: Failed to get areaID");
        return false;
    }

    time_t timep = time(NULL);
    service_bean_.installTime = timep;

    boost::shared_ptr<TSocket> socket(new TSocket(server_ip_.c_str(), stoul(server_port_)));
    socket->setConnTimeout(1000 * 5); // set connection timeout 5S
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ConfigServiceClient client(protocol);
    try
    {
        transport->open();
        ServiceConfigResult result;
        int i = 0;
        do
        {
            //ReadConfig();
            client.registerService(result, service_bean_);
            i++;
        }while(result.property == 4 && i < 3);

        if(result.property == 1 || result.property == 2)
        {
            heart_id_ = result.ID;
            WriteConfig();
            LOG_INFO("RegistToConfSrv: register service success. ip:" << service_bean_.ip << ", port:" << service_bean_.port);
        }
        else
        {
            ret = false;
            LOG_ERROR("RegistToConfSrv: register service failed: " << result.property);
        }
        transport->close();
    }
    catch(TException & tx)
    {
        LOG_ERROR("RegistToConfSrv: thrift exception:" << tx.what());
        ret = false;
    }
    return ret;
}

bool ServiceReg::GetServiceIp(const std::string& service_code, const std::string& org_id, std::string& ip, std::string& port)
{
    if(service_code.empty())
    {
        LOG_ERROR("GetServiceIp: service code is empty");
        return false;
    }

    vector<ServiceConfigBean> service_info;
    std::string version = "1.0";
    ParseConfigure::GetInstance().GetProperty("service.version", version);

    QueryService(service_info, service_code, version);

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

string ServiceReg::RequestService(const std::string& ip, const std::string& port, const std::string& maxcode, const std::string& mincode, const bool& bzip, std::string& jdata)
{
    string res;
    boost::shared_ptr<TSocket> socket(new TSocket(ip, stoul(port)));
    socket->setConnTimeout(1000 * 5); // set connection timeout 5S
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    CommonServiceClient client(protocol);
    try
    {
        transport->open();

        std::string  key = GenerteKey();
        unsigned int flag = GetRandomInteger(1, 30);
        ST_ENCRYPT encrypt;
        EncryptMode(flag, key, encrypt);
        CGenAlgori  genrial;
        std::string encrypt_data = genrial.EnCrypt(jdata, encrypt.mode, encrypt.szKey);

        string checkcode = calCRC(encrypt_data);
        client.getDataTS(res, maxcode, mincode, checkcode, bzip, encrypt_data, true, key, flag);

        std::string decrypt_data = genrial.DeCrypt(res, encrypt.mode, encrypt.szKey);
        res = decrypt_data;

        transport->close();
    }
    catch(TException & tx)
    {
        LOG_ERROR("RequestService: thrift exception:" << tx.what());
    }
    return res;
}

void ServiceReg::QueryService(std::vector<ServiceConfigBean>& service_info, const std::string& service_code, const std::string& version)
{
    if(server_ip_.empty() || server_port_.empty())
    {
        LOG_ERROR("QueryService: server ip or port is empty");
        return;
    }

    boost::shared_ptr<TSocket> socket(new TSocket(server_ip_.c_str(), stoul(server_port_)));
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
        LOG_ERROR("QueryService: thrift exception:" << tx.what());
    }
    return;
}

void ServiceReg::StartHeartThead()
{
    heart_thread_ = std::thread(&ServiceReg::StartHeartBeat, this);
}

void ServiceReg::StopHeartThead()
{
    heart_status_ = false;
    if(heart_thread_.joinable())
    {
        heart_thread_.join();
        LOG_INFO("StopHeartThead: heart beat has stoped!");
    }
}

void ServiceReg::StartHeartBeat()
{
    bool res = false;
    int i = 0;
    while(heart_status_)
    {
        if(i == 2)
        {
            break;
        }
        res = HeartBeat();
        if(!res)
        {
            i++;
        }
        else
        {
            i = 0;
        }
        sleep(3);
    }

    if(i == 2 && heart_status_ == true)
    {
        LOG_ERROR("StartHeartBeat: heart beat failed");
        //重启服务
    }
    return;
}

bool ServiceReg::HeartBeat()
{
    bool ret = true;
    boost::shared_ptr<TSocket> socket(new TSocket(server_ip_.c_str(), stoul(server_port_)));
    socket->setConnTimeout(1000 * 5);
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ConfigServiceClient client(protocol);

    try
    {
        transport->open();
        if(!service_bean_.SSID.empty())
        {
            int8_t result = client.serviceHeart(service_bean_.SSID);
            if(result != 1 && result != 2)
            {
                LOG_ERROR("HeartBeart error: result=" << result);
                ret = false;
            }
        }
        else
        {
            LOG_ERROR("HeartBeart: service SSID is empty, please register service.");
            ret = false;
        }
        transport->close();
    }
    catch(TException& tx)
    {
        LOG_ERROR("HeartBeart: thrift exception:" << tx.what());
        ret = false;
    }
    return ret;
}

std::string ServiceReg::GenericSSID()
{
    std::string sid;
    time_t now;
    localtime(&now);
    char buffer[100] = {0};
    sprintf(buffer, "%ld", now);
    sid = GetStrMd5((char*)buffer, strlen(buffer));
    return sid;
}

std::string ServiceReg::GetStrMd5(char* szSrc, int iLen)
{
    unsigned char szmd5sum[16];
    md5_context ctx;
    md5_starts(&ctx);
    md5_update(&ctx, (unsigned char *)szSrc, iLen);
    md5_finish(&ctx, szmd5sum);

    char szMd5[260] = {0};
    char szChar[3];
    for(int i = 0; i < 16; i ++)
    {
        itoa(szmd5sum[i], szChar, 16);
        if(strlen(szChar) == 1)
        {
            strcat((char*)szMd5,  "0");
        }
        strcat((char*)szMd5,  szChar);
    }

    std::string szResult = szMd5;

    return szResult;
}
}}}
