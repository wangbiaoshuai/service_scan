#include "service_reg.h"

#include <log.h>
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TBinaryProtocol.h>
#include <sys/time.h>

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
    service_bean_.port = atoi(value.c_str());

    key = "service.code";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.serviceID);

    key = "service.version";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.version);

    service_bean_.orgID = server_areaID_;
    /*key = "service.serverAreaId";
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.orgID);*/

    service_bean_.SSID = GenericSSID();

    service_bean_.__isset.serverID = true;
    service_bean_.serverID  = GenericUUID(service_bean_.ip);

    key = "service.name";
    service_bean_.__isset.name = true;
    ParseConfigure::GetInstance().GetProperty(key, service_bean_.name);
   
    service_bean_.__isset.location = true;
    service_bean_.location = GetCurrentPath();

    key = "service.description";
    service_bean_.__isset.description = true;
    service_bean_.description = SERVICE_SCAN_DESC;
    //ParseConfigure::GetInstance().GetProperty(key, service_bean_.description);

    struct timeval timer;
    gettimeofday(&timer, NULL);
    int64_t current_time = timer.tv_sec * 1000 + timer.tv_usec / 1000;
    service_bean_.__isset.installTime = true;
    service_bean_.installTime = current_time;

    LOG_DEBUG("ServiceReg::ReadConfig success!");
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
    time_t time = service_bean_.installTime / 1000;
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

bool ServiceReg::Start()
{
    if(!IsLegalAddr())
    {
        LOG_ERROR("service ip is illegal");
        return false;
    }
    bool res = false;
    do
    {
        res = RegistToConfSrv();
        if(res)
        {
            break;
        }
        else
        {
            LOG_ERROR("Start: register to configure service failed. sleep 60S...");
            sleep(60);
        }
    }while(true);

    //注册成功之后创建心跳线程
    if(StartHeartThead() != 0)
    {
        LOG_ERROR("Start: StartHeartThead error");
        return false;
    }
    return true;
}

bool ServiceReg::Stop()
{
    //停止心跳线程
    if(StopHeartThead() != 0)
    {
        return false;
    }
    return true;
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

std::string ServiceReg::GetAreaId() //目前没有使用,接口已经测试无问题
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
    if(Fetch(max_code, "", addr_ip, addr_port) == false)
    {
        LOG_ERROR("GetAreaId: Fetch failed");
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
    LOG_INFO("begin to register to " << server_ip_ << ":" << server_port_);
    bool ret = true;

    boost::shared_ptr<TSocket> socket(new TSocket(server_ip_.c_str(), atoll(server_port_.c_str())));
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
            LOG_INFO("RegistToConfSrv: register service success. heartID:" << heart_id_);
        }
        else
        {
            ret = false;
            LOG_ERROR("RegistToConfSrv: register service failed: " << (int)result.property);
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

bool ServiceReg::Fetch(const std::string& service_code, const std::string& org_id, std::string& ip, std::string& port)
{
    if(service_code.empty())
    {
        LOG_ERROR("Fetch: service code is empty");
        return false;
    }

    vector<ServiceConfigBean> service_info;
    std::string version = "1.0"; //版本号必须是1.0，否则查询不到服务

    QueryService(service_info, service_code, version);

    if(service_info.empty())
    {
        LOG_ERROR("Fetch: can not query service:" << service_code << " info.");
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

string ServiceReg::RequestService(const std::string& ip, const std::string& port, const std::string& maxcode, const std::string& mincode, const bool& bzip, std::string& jdata)
{
    string res;
    boost::shared_ptr<TSocket> socket(new TSocket(ip, atoll(port.c_str())));
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
    LOG_INFO("Begin query service:" << service_code << " info.");
    if(server_ip_.empty() || server_port_.empty())
    {
        LOG_ERROR("QueryService: server ip or port is empty");
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
        LOG_ERROR("QueryService: thrift exception:" << tx.what());
    }
    return;
}

void* thread_function(void* contex)
{
    ServiceReg* srv = (ServiceReg*)contex;
    srv->StartHeartBeat();
    pthread_exit(NULL);
}

int ServiceReg::StartHeartThead()
{
    int res = pthread_create(&heart_thread_, NULL, thread_function, this);
    if(res != 0)
    {
        LOG_ERROR("StartHeartThead: create heartbeat thread failed");
    }
    return res;
}

int ServiceReg::StopHeartThead()
{
    heart_status_ = false;
    int res = pthread_join(heart_thread_, NULL);
    if(res != 0)
    {
        LOG_ERROR("StopHeartThead: heartbeat thread join failed");
        return res;
    }
    LOG_INFO("StopHeartThead: heartbeat thread has been joined");
    return res;
}

bool ServiceReg::ReregisterService()
{
    bool res = false;
    do
    {
        res = RegistToConfSrv();
        if(res)
        {
            break;
        }
        else
        {
            LOG_ERROR("ReregisterService: register to configure service failed. sleep 60S...");
            sleep(60);
        }
    }while(true);

    return true;
}


void ServiceReg::StartHeartBeat()
{
    LOG_INFO("StartHeartBeat: heartbeat thread start...");
    bool res = false;
    int i = 0;
    while(heart_status_)
    {
        if(i == 3)
        {
            LOG_WARN("StartHeartBeat: heartbeat failed. begin re-register service...");
            ReregisterService();
            LOG_INFO("StartHeartBeat: re-register service success.");
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

    LOG_INFO("StartHeartBeat: heartbeat thread end.");
    return;
}

bool ServiceReg::HeartBeat()
{
    bool ret = true;
    boost::shared_ptr<TSocket> socket(new TSocket(server_ip_.c_str(), atoll(server_port_.c_str())));
    socket->setConnTimeout(1000 * 5);
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ConfigServiceClient client(protocol);

    try
    {
        transport->open();
        if(!heart_id_.empty())
        {
            int result = client.serviceHeart(heart_id_);
            if(result != 1 && result != 2)
            {
                LOG_ERROR("HeartBeart error: result=" << result);
                ret = false;
            }
            LOG_DEBUG("heart info: " << heart_id_ << ", result: " << result);
        }
        else
        {
            LOG_ERROR("HeartBeart: service heartID is empty, please register service.");
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
    string sid;
    timeval tv;
    gettimeofday(&tv, NULL);
    long time = tv.tv_sec * 1000 * 1000 + tv.tv_usec;
    char buffer[100] = {0};
    sprintf(buffer, "%ld", time);
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
