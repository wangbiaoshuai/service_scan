#include "dev_manager.h"
#include "arp.h"
#include "service_reg.h"
#include "defines.h"
#include "log.h"
#include "common_function.h"
#include "json/json.h"

#define SCAN_UNREPLY_COUNT	3

DevManager::DevManager():
report_block_(),
report_devreg_(),
unregister_dev_(),
unregister_dev_keep_(),
register_dev_(),
register_dev_keep_(),
roaming_dev_(),
detect_mode_(0),
nmap_dev_mutex_(),
nmap_dev_()
{
}

DevManager::~DevManager()
{
    report_block_.Close();
    report_devreg_.Close();
}

void DevManager::SetDetectMode(int mode)
{
    detect_mode_ = mode;
}

bool DevManager::GetDevfromNmap(DEV_INFO& device)
{
    bool res = false;
    nmap_dev_mutex_.Lock();
    do
    {
        if(nmap_dev_.empty())
        {
            break;
        }

        device = nmap_dev_.front();
        nmap_dev_.pop_front();
        res = true;
    }while(0);
    nmap_dev_mutex_.Unlock();
    return res;
}

void DevManager::ClearNmap()
{
    list<DEV_INFO> ().swap(nmap_dev_);
}

int DevManager::ReportNmapDev(const DEV_INFO& device)
{
    std::string szText = CreateSendText(device);
    LOG_DEBUG("ReportNmapDev: discover unregister device: "<<szText.c_str());
    bool bret;
    bret = report_devreg_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
    if(!bret)
    {
        LOG_ERROR("ReportNmapDev: send data to center service failed. data:"<<szText);
    }

    bret = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
    if(!bret)
    {
        LOG_ERROR("ReportNmapDev: send data to block service failed. data:"<<szText);
    }
    return 0;
}

void DevManager::ClearDevMap()
{
    mapDev ().swap(register_dev_);
    mapDev ().swap(register_dev_keep_);
    mapDev ().swap(unregister_dev_);
    mapDev ().swap(unregister_dev_keep_);
    mapDev ().swap(roaming_dev_);
}

int DevManager::GetDevRegIp()
{
    ServiceReg cm;

    std::string szDevRegIp;
    std::string szDevRegPort;
    std::string szDevRegOrgId;

    if(cm.Fetch(SERVICE_CODE_CENTER, szDevRegOrgId, szDevRegIp, szDevRegPort))
    {
        LOG_DEBUG("GetDevRegIp: fetch block service ip=" << szDevRegIp << ", port=" << szDevRegPort);
    }
    else
    {
        LOG_ERROR("GetDevRegIp: fetch devreg service error.");
    }

    if(report_devreg_.Init(szDevRegIp, atoi(szDevRegPort.c_str()), 1) == false)
    {
        return -1;
    }
    return 0;
}

int DevManager::GetBlockIp()
{
    ServiceReg cm;

    std::string szBlockIp;
    std::string szBlockPort;
    std::string szBlockOrgId;

    if(cm.Fetch(SERVICE_CODE_BLOCK, szBlockOrgId, szBlockIp, szBlockPort))
    {
        LOG_DEBUG("GetBlockIp: fetch block service ip=" << szBlockIp << ", port=" << szBlockPort);
    }
    else
    {
        LOG_ERROR("GetBlockIp: fetch block service error.");
    }

    if(report_block_.Init(szBlockIp, atoi(szBlockPort.c_str()), 1) == false)
    {
        return -1;
    }
    return 0;
}

int DevManager::InitReportObj()
{
    int res = 0;
    GetDevRegIp();
    GetBlockIp();
    //从这里开始上报数据，所以需要打开连接。
    if(!report_devreg_.Open())
    {
        res = -1;
        LOG_ERROR("InitReportObj: open devreg report error.");
    }

    if(!report_block_.Open())
    {
        res = -1;
        LOG_ERROR("InitReportObj: open block report error.");
    }
    return res;
}

void DevManager::UpdateRegKeepDev()
{
    mapDev::iterator iter;
    for(iter = register_dev_.begin(); iter != register_dev_.end(); ++iter)
    {
        register_dev_keep_.insert(mapDev::value_type(iter->first, iter->second));
    }
}

void DevManager::PushUnregDev(unsigned long ip, const DEV_INFO& device)
{
    if(register_dev_.find(ip) != register_dev_.end())
    {
        unregister_dev_.erase(ip);
        unregister_dev_keep_.erase(ip);
        return;
    }

    unregister_dev_.insert(mapDev::value_type(ip, device));
    mapDev::iterator iter = unregister_dev_keep_.find(ip);
    if(iter != unregister_dev_keep_.end())
    {
        if(iter->second.szReportRes.RepSuccess())
        {
            return;
        }
        unregister_dev_.insert(mapDev::value_type(iter->first, iter->second));
    }

    if(unregister_dev_[ip].szMac.empty())
    {
        string mac;
        struct in_addr sin_addr;
        sin_addr.s_addr = htonl(ip);
        string tmp_ip = inet_ntoa(sin_addr);
        get_mac_addr(tmp_ip, mac);
        unregister_dev_[ip].szMac = mac;
    }

    bool bret = false;
    std::string szText = CreateSendText(unregister_dev_[ip]);
    if(unregister_dev_[ip].szReportRes.is_report_block == false)
    {
        LOG_DEBUG("find unregister device, report block service: "<<szText);
        bret = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText); 
        if(!bret)
        {
            LOG_ERROR("PushUnregDev: send data to center service failed. data:"<<szText);
        }
        else
        {
            unregister_dev_[ip].szReportRes.is_report_block = true;
        }
    }

    if(unregister_dev_[ip].szReportRes.is_report_devreg == false)
    {
        LOG_DEBUG("find unregister device, report devreg service: "<<szText);
        bret = report_devreg_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
        if(!bret)
        {
            LOG_ERROR("parseClientPack: send data to center service failed. data:"<<szText);
        }
        else
        {
            unregister_dev_[ip].szReportRes.is_report_devreg = true;
        }
    }

    unregister_dev_keep_.insert(mapDev::value_type(ip, unregister_dev_[ip]));
}

void DevManager::PushRoamingDev(unsigned long ip, const DEV_INFO& device)
{
    mapDev::iterator iter = roaming_dev_.find(ip);
    if(iter != roaming_dev_.end())
    {
        if(iter->second.szReportRes.RepSuccess())
        {
            iter->second.count = 0;
            return;
        }
    }
    else
    {
        roaming_dev_.insert(mapDev::value_type(ip, device));
    }

    if(detect_mode_ == 1)
    {
        nmap_dev_mutex_.Lock();
        nmap_dev_.push_back(roaming_dev_[ip]);
        nmap_dev_mutex_.Unlock();
    }

    if(roaming_dev_[ip].szMac.empty())
    {
        string mac;
        struct in_addr sin_addr;
        sin_addr.s_addr = htonl(ip);
        string tmp_ip = inet_ntoa(sin_addr);
        get_mac_addr(tmp_ip, mac);
        roaming_dev_[ip].szMac = mac;
    }
    std::string szText = CreateSendText(roaming_dev_[ip]);
    bool bret = false;
    if(roaming_dev_[ip].szReportRes.is_report_block == false)
    {
        LOG_DEBUG("find roaming device, report to block service: "<<szText);
        bret = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText); 
        if(!bret)
        {
            LOG_ERROR("parseClientPack: send data to center service failed. data:"<<szText);
        }
        else
        {
            roaming_dev_[ip].szReportRes.is_report_block = true;
        }
    }

    if(roaming_dev_[ip].szReportRes.is_report_devreg == false)
    {
        LOG_DEBUG("find roaming device, report devreg service: "<<szText);
        bret = report_devreg_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
        if(!bret)
        {
            LOG_ERROR("parseClientPack: send data to center service failed. data:"<<szText);
        }
        else
        {
            roaming_dev_[ip].szReportRes.is_report_devreg = true;
        }
    }
}

int DevManager::CalculateCloseDev()
{
    mapDev::iterator it = register_dev_keep_.begin();
    while(it != register_dev_keep_.end())
    {
        if(register_dev_.find(it->first) == register_dev_.end() 
           && unregister_dev_.find(it->first) == unregister_dev_.end())
        {
            it->second.count ++;
            if(it->second.count < SCAN_UNREPLY_COUNT)
            {
                ++it;
                continue;
            }

            //检测到已注册设备->关机设备。
            it->second.szBoot = "0";
            std::string szText = CreateSendText(it->second);
            LOG_DEBUG("DetectClose: discover register->shutdown device: "<<szText.c_str());

            bool bret;
            bret = report_devreg_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to center service error, data:"<<szText.c_str());
            }

            bret = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to block service error, data:"<<szText.c_str());
            }
            register_dev_keep_.erase(it++);
            continue;
        }
        ++it;
    }

    it = unregister_dev_keep_.begin();
    while(it != unregister_dev_keep_.end())
    {
        if(register_dev_.find(it->first) == register_dev_.end()
           && unregister_dev_.find(it->first) == unregister_dev_.end())
        {
            it->second.count ++;
            if(it->second.count < SCAN_UNREPLY_COUNT)
            {
                ++it;
                continue;
            }
            
            //检测到未注册设备->关机设备
            it->second.szBoot = "0";
            std::string szText = CreateSendText(it->second);
            LOG_DEBUG("DetectClose: discover unregister->shutdown device: "<<szText.c_str());

            bool bret;
            bret = report_devreg_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to center service error, data:"<<szText.c_str());
            }

            bret = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to block service error, data:"<<szText.c_str());

            }
            unregister_dev_keep_.erase(it++);
            continue;
        }
        ++it;
    }

    mapDev ().swap(register_dev_);
    mapDev ().swap(unregister_dev_);

    return 0;
}

void DevManager::PushRegDev(unsigned long ip, const DEV_INFO& device)
{
    register_dev_.insert(mapDev::value_type(ip, device));
    mapDev::iterator iter = unregister_dev_keep_.find(ip);
    if(iter != unregister_dev_keep_.end())
    {
        unregister_dev_keep_.erase(iter);
    }
}

std::string DevManager::CreateSendText(const DEV_INFO& info)
{
    Json::Value root;
    Json::Value value;

    Json::FastWriter writer;
    std::string szText;

    struct tm *ptm;
    long ts = time(NULL);

    ptm = localtime(&ts);

    char tmBuf[100] = {0};
    sprintf(tmBuf, "%04d-%02d-%02d %02d:%02d:%02d:", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);


    value["devType"] = info.szDevType;
    value["osType"] = info.szOsType;
    value["ip"] = info.szIP;
    value["groupName"] = info.szGroupName;
    value["hostName"] = info.szHostName;
    value["mac"] = info.szMac;
    value["devOnlyId"] = info.szDevId;
    value["isOpened"] = info.szBoot;
    value["isFireWall"] = info.szFireWall;
    value["orgId"] = info.szOrgId;
    value["regOrgId"] = info.szRegOrgId;
    value["areaId"] = info.szAreaId;
    value["regAreaId"] = info.szRegAreaId;
    value["clientTime"] = tmBuf;

    root.append(value);
    szText = writer.write(root);

    return szText;
}
