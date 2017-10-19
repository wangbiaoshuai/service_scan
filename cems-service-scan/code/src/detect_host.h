#ifndef _DETECT_HOST_H
#define _DETECT_HOST_H

#include "defines.h"
#include "upreport.h"
#include <netinet/ip_icmp.h>

namespace cems{ namespace service{ namespace scan{
enum SEND_TYPE
{
    s_ping = 0,
    s_nbt,
    s_reg,
};

struct _NBT_INFO
{
    std::string szIp;
    std::string szMac;
    std::string szWorkName;
    std::string szGroupName;
};

class DetectHost
{
public:
    DetectHost();
    ~DetectHost();

    int Init(int mode = 1);
    int Start(MAP_COMMON * ipRange, std::string szAreaId, std::string szOrgId);
    void DetectChange();
    int CalculateCloseDev();
    void Close();

private:
    int DepthScan();
    int StandardScan(MAP_COMMON * ipRange);
    int SendPacks( char* start, char* end, SEND_TYPE type);
    bool SendPack(unsigned long uip, SEND_TYPE type); 
    std::string CreateSendText(DEV_INFO & info);

public:
    int SendDetectPack(MAP_COMMON* ipRange, SEND_TYPE send_type);
    void RecvClientPack();
    bool RecvIcmpPack();
    void RecvNbtPack();

private:
    int ClientProbe(MAP_COMMON * ipRange);
    void MakeClientPack(char* sendBuffer, int & len);
    void ParseClientPack(ULONG uip, char* data, int iLen);

private:
    int IcmpProbe(MAP_COMMON* const ipRange);
    bool ParseIcmpPack(char *buf,int len);
    int MakeIcmpPack(int pack_no, struct icmp* icmp);

private:
    int NbtProbe(MAP_COMMON* const ipRange);
    void ParseNbtPack(ULONG uip, char* buffer, ULONG uLen,  _NBT_INFO* pinfo);
    void MakeQueryPack(struct Q_NETBIOSNS& nbns);

private:
    int detect_mode_;
    //Mutex unregister_mutex_;
    //Mutex register_mutex_;

    UpReport report_block_;
    UpReport report_center_;

    mapDev unregister_dev_;
    mapDev unregister_dev_keep_;

    mapDev register_dev_;
    mapDev register_dev_keep_;
    mapDev roaming_dev_;

    std::string area_id_;
    std::string org_id_;

    volatile int register_scan_stop_;
    volatile int icmp_scan_stop_;
    volatile int nbt_scan_stop_;
    int pid_;
    int sockfd_;
    int sockud_;
    int sockcd_;
};
}}}
#endif // _DETECT_HOST_H