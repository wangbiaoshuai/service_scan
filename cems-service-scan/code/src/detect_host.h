#ifndef _DETECT_HOST_H
#define _DETECT_HOST_H

#include "dev_manager.h"
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

    int Init();
    int Start(MAP_COMMON * ipRange, std::string szAreaId, std::string szOrgId, int mode = 0);
    void DetectChange();
    int CalculateCloseDev();
    void Close();

private:
    int StandardScan(MAP_COMMON * ipRange);
    int SendPacks( char* start, char* end, SEND_TYPE type);
    bool SendPack(unsigned long uip, SEND_TYPE type); 
    std::string CreateSendText(DEV_INFO & info);

public:
    int SendDetectPack(MAP_COMMON* ipRange, SEND_TYPE send_type);
    void RecvClientPack();
    bool RecvIcmpPack();
    void RecvNbtPack();
    int DepthScan();

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
    DevManager dev_manager_;

    std::string area_id_;
    std::string org_id_;

    volatile int register_scan_stop_;
    volatile int icmp_scan_stop_;
    volatile int nbt_scan_stop_;
    int pid_;
    int sockfd_;
    int sockud_;
    int sockcd_;
    pthread_t dep_scan_thread_;
    volatile int dep_scan_stop_;
    volatile int dep_scan_pause_;
};
}}}
#endif // _DETECT_HOST_H
