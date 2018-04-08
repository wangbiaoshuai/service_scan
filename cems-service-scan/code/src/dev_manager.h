#ifndef _DEV_MANAGER_H
#define _DEV_MANAGER_H

#include "mutex.h"
#include "upreport.h"

using namespace cems::service::scan;
struct ReportResult
{
    bool is_report_block;
    bool is_report_devreg;

    ReportResult():
    is_report_block(false),
    is_report_devreg(false)
    {
    }

    bool RepSuccess()
    {
        if(is_report_block == true && is_report_devreg == true)
        {
            return true;
        }
        return false;
    }

    struct ReportResult operator=(const struct ReportResult& obj)
    {
        this->is_report_block = obj.is_report_block;
        this->is_report_devreg = obj.is_report_devreg;
        return *this;
    }
};

struct  DEV_INFO
{
    std::string szDevType;
    std::string szOsType;
    std::string szIP;
    std::string szGroupName;
    std::string szHostName;
    std::string szMac;
    std::string szBoot;
    std::string szDevId;
    std::string szFireWall;
    std::string szOrgId;
    std::string szRegOrgId;
    std::string szAreaId;
    std::string szRegAreaId;
    ReportResult szReportRes;
    int    count;
    
    DEV_INFO(): 
    szDevType(""),
    szOsType(""),
    szGroupName(""),
    szHostName(""),
    szMac(""),
    szBoot(""),
    szDevId(""),
    szFireWall(""),
    szOrgId(""),
    szRegOrgId(""),
    szAreaId(""),
    szRegAreaId(""),
    szReportRes(),
    count(0)
    {
    }

    struct DEV_INFO operator=(const struct DEV_INFO& device)
    {
        this->szDevType = device.szDevType;
        this->szOsType = device.szOsType;
        this->szIP = device.szIP;
        this->szGroupName = device.szGroupName;
        this->szHostName = device.szHostName;
        this->szMac = device.szMac;
        this->szBoot = device.szBoot;
        this->szDevId = device.szDevId;
        this->szFireWall = device.szFireWall;
        this->szOrgId = device.szOrgId;
        this->szRegOrgId = device.szRegOrgId;
        this->szAreaId = device.szAreaId;
        this->szRegAreaId = device.szRegAreaId;
        this->szReportRes = device.szReportRes;
        this->count = device.count;
        return *this;
    }
};

typedef std::map<unsigned long, DEV_INFO> mapDev;

class DevManager
{
public:
    DevManager();
    ~DevManager();

    int InitReportObj();

    void PushRoamingDev(unsigned long ip, const DEV_INFO& device);
    void PushRegDev(unsigned long ip, const DEV_INFO& device);
    void PushUnregDev(unsigned long ip, const DEV_INFO& device);
    void ClearDevMap();

    void UpdateRegKeepDev();
    int CalculateCloseDev();

    void SetDetectMode(int mode);

    bool GetDevfromNmap(DEV_INFO&);
    int ReportNmapDev(const DEV_INFO&);
    void ClearNmap();

private:
    std::string CreateSendText(const DEV_INFO& info);
    int GetDevRegIp();
    int GetBlockIp();

private:
    UpReport report_block_;
    UpReport report_devreg_;

    mapDev unregister_dev_;
    mapDev unregister_dev_keep_;

    mapDev register_dev_;
    mapDev register_dev_keep_;
    mapDev roaming_dev_;

    int detect_mode_;  //0是标准扫描，1是深度扫描
    Mutex nmap_dev_mutex_;
    std::list<DEV_INFO> nmap_dev_;

};
#endif //_DEV_MANAGER_H
