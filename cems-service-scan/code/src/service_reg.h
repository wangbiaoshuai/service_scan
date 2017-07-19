#ifndef SERVICE_REG_H_
#define SERVICE_REG_H_

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include "ConfigService.h"

namespace cems{ namespace service{ namespace scan{

using namespace com::vrv::im::service;

#define SERVICE_CODE_ADDRESS "00FF0600"
#define MINCODE_ADDRESS_AREAID "8"
#define LOG_CONFIG_PATH "../config/log4cplus.properties"

class ServiceReg
{
public:
    ServiceReg();
    ~ServiceReg();

    bool IsLegalAddr();
    bool RegistToConfSrv();
    void StartHeartThead();
    void StopHeartThead();
    int32_t GetServicePort();

private:
    bool ReadConfig();
    void WriteConfig();
    std::string GetAreaId();
    void StartHeartBeat();
    bool HeartBeat();
    std::string RequestService(const std::string& ip, const std::string& port, const std::string& maxcode, const std::string& mincode, const bool& bzip, std::string& jresult);

    bool GetServiceIp(const std::string& service_code, const std::string& org_id, std::string& ip, std::string& port);

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
    std::atomic<bool> heart_status_;
    std::thread heart_thread_;
};
}}}
#endif // SERVICE_REG_H_
