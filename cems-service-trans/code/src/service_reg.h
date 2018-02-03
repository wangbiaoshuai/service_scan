#ifndef SERVICE_REG_H_
#define SERVICE_REG_H_

#include <string>
#include <vector>
#include "ConfigService.h"
#include "config_types.h"

namespace cems{ namespace service{ namespace scan{

using namespace com::vrv::im::service;

#define SERVICE_CODE_ADDRESS "00FF0600"
#define MINCODE_ADDRESS_AREAID "8"
#define LOG_CONFIG_PATH "../config/log4cplus.properties"
#define SERVICE_SCAN_DESC "\u4e2d\u8f6c\u670d\u52a1"

class ServiceReg
{
public:
    ServiceReg();
    ~ServiceReg();

    bool Start();
    bool Stop();
    int32_t GetServicePort();
    void StartHeartBeat();
    bool RegistToConfSrv();
    bool Fetch(const std::string& service_code, const std::string& org_id, std::string& ip, std::string& port);
    std::string RequestService(const std::string& ip, const std::string& port, const std::string& maxcode, const std::string& mincode, const bool& bzip, std::string& jdata);

private:
    bool IsLegalAddr();
    bool ReregisterService();
    int StartHeartThead();
    int StopHeartThead();
    
private:
    bool ReadConfig();
    void WriteConfig();
    std::string GetAreaId();
    bool HeartBeat();
    void QueryService(std::vector<ServiceConfigBean>& service_info, const std::string& service_code, const std::string& version);

private:
    std::string GenericSSID();
    std::string GetStrMd5(char* szSrc, int iLen);

private:
    ServiceConfigBean service_bean_;
    std::string server_ip_;
    std::string server_port_;
    std::string server_areaID_;
    std::string heart_id_;
    bool heart_status_;
    pthread_t heart_thread_;
};
}}}
#endif // SERVICE_REG_H_
