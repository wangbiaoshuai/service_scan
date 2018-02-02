#ifndef _SERVICE_REQUEST_H
#define _SERVICE_REQUEST_H

#include <string>
#include <vector>
#include <map>
#include "ConfigService.h"
#include "config_types.h"
#include "CommonService.h"
#include "mutex.h"
#include "bean_types.h"
extern "C"{
#include <ngx_http.h>
}

using namespace com::vrv::im::service;
using namespace com::vrv::cems::service::base::bean::cache;

#define MAP_MAX_SIZE 500
#define SERVICE_CACHE_MAXCODE "00FF0700"
#define RPC_CONN_TIMEOUT 5
#define RPC_RECV_TIMEOUT 10

typedef struct ParamInfo
{
    std::string maxcode_;
    std::string mincode_;
    std::string checkcode_;
    std::string session_id_;
    unsigned int msgcode_;
    bool is_zip_;
    std::string jdata_;
    bool is_encrypt_;
    std::string key_;
    int32_t flag_;

    ParamInfo():
    maxcode_(""),
    mincode_(""),
    checkcode_(""),
    msgcode_(0),
    is_zip_(false),
    jdata_(""),
    is_encrypt_(false),
    key_(""),
    flag_(0)
    {
    }
} PARAM_INFO;

typedef struct Address
{
    std::string ip_;
    std::string port_;

    Address():
    ip_(""),
    port_("")
    {
    }
} Address;

class ServiceRequest
{
public:
    ServiceRequest();
    ~ServiceRequest();
    int SetAddr(const std::string& ip, const std::string& port);
    int ParseMsgBody(const void* msg, unsigned int len, PARAM_INFO& param_info);
    std::string GetdataTC(const PARAM_INFO& param_info);
    void SetLog(ngx_log_t* log);

private:
    bool Fetch(const std::string& service_code, std::string& ip, std::string& port);
    std::string GetdataTC(const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& jdata, const std::string& session_id, const int32_t msg_code);
    void QueryService(std::vector<ServiceConfigBean>& service_info, const std::string& service_code, const std::string& version);
    int QueryDeviceKeyByCache(const std::string session_id, DeviceKeyCache& device);

private:
    std::string server_ip_;
    std::string server_port_;
    Mutex session_map_lock_;
    std::map<std::string, DeviceKeyCache> session_map_;
    Mutex service_addr_map_lock_;
    std::map<std::string, Address> service_addr_map_;
    ngx_log_t* log_;
};

#endif // _SERVICE_REQUEST_H
