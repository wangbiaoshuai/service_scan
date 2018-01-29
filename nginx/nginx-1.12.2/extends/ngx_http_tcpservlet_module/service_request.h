#ifndef _SERVICE_REQUEST_H
#define _SERVICE_REQUEST_H

#include <string>
#include <vector>
#include "ConfigService.h"
#include "config_types.h"
#include "CommonService.h"

#define SERVICE_CODE_ADDRESS "00FF0600"
#define MINCODE_ADDRESS_AREAID "8"

using namespace com::vrv::im::service;

class ServiceRequest
{
public:
    ServiceRequest(const std::string& ip, const std::string& port);
    ~ServiceRequest();

    bool Fetch(const std::string& service_code, std::string& ip, std::string& port);
    std::string GetdataTS(const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& jdata, const bool isEncrypt, const std::string& key, const int32_t flag);

private:
    void QueryService(std::vector<ServiceConfigBean>& service_info, const std::string& service_code, const std::string& version);

private:
    std::string server_ip_;
    std::string server_port_;
};

#endif // _SERVICE_REQUEST_H
