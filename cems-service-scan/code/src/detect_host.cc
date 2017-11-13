#include "detect_host.h"

#include "json/json.h"
#include "conv.h"
#include "common_function.h"
#include "log.h"
#include "service_reg.h"
#include "arp.h"

using namespace cems::service::scan;
using namespace std;

#define THREAD_SLEEP_TIME	5*1000*1000
#define SOCKET_SELECT_TIMEOUT	3*1000*1000
#define PACKET_SIZE     4096
#define SCAN_UNREPLY_COUNT	3
static const int g_datalen = 40;

struct timeval tvSub(struct timeval timeval1,struct timeval timeval2);
bool getsockaddr(const char * hostOrIp, struct sockaddr_in* sockaddr);
unsigned short getChksum(unsigned short *addr,int len);

typedef struct
{
    DetectHost* context;
    MAP_COMMON* ip_range;
    SEND_TYPE send_type;
}SendPackParam;

DetectHost::DetectHost():
detect_mode_(1),
//unregister_mutex_(),
//register_mutex_(),
report_block_(),
report_center_(),
unregister_dev_(),
unregister_dev_keep_(),
register_dev_(),
register_dev_keep_(),
roaming_dev_(),
area_id_(""),
org_id_(""),
register_scan_stop_(0),
icmp_scan_stop_(0),
nbt_scan_stop_(0),
pid_(0),
sockfd_(0),
sockud_(0),
sockcd_(0)
{
}

DetectHost::~DetectHost()
{
}

int DetectHost::Init(int mode)
{
    LOG_INFO("DetectInit: begin.");
    detect_mode_ = mode;

    GetDataCenterIp();
    GetBlockIp();

    LOG_INFO("Init: end.");
    return 	1;
}

int DetectHost::GetDataCenterIp()
{
    ServiceReg cm;

    std::string szCenterIp;
    std::string szCenterPort;
    std::string szCenterOrgId;

    if(cm.Fetch(SERVICE_CODE_CENTER, szCenterOrgId, szCenterIp, szCenterPort))
    {
        LOG_DEBUG("GetDataCenterIp: fetch data service ip=" << szCenterIp << ", port=" << szCenterPort);
    }
    else
    {
        LOG_ERROR("GetDataCenterIp: fetch data service error.");
    }

    if(report_center_.Init(szCenterIp, atoi(szCenterPort.c_str()), 1) == false)
    {
        return -1;
    }
    return 0;
}

int DetectHost::GetBlockIp()
{
    ServiceReg cm;

    std::string szBlockIp;
    std::string szBlockPort;
    std::string szBlockOrgId;

    if(cm.Fetch(SERVICE_CODE_BLOCK, szBlockOrgId, szBlockIp, szBlockPort))
    {
        LOG_DEBUG("Init: fetch block service ip=" << szBlockIp << ", port=" << szBlockPort);
    }
    else
    {
        LOG_ERROR("Init: fetch block service error.");
    }

    if(report_block_.Init(szBlockIp, atoi(szBlockPort.c_str()), 1) == false)
    {
        return -1;
    }
    return 0;
}

void DetectHost::DetectChange()
{
    mapDev ().swap(register_dev_);
    mapDev ().swap(register_dev_keep_);
    mapDev ().swap(unregister_dev_);
    mapDev ().swap(unregister_dev_keep_);
    mapDev ().swap(roaming_dev_);
}

int DetectHost::Start(MAP_COMMON * ipRange, std::string szAreaId, std::string szOrgId)
{
    //从这里开始上报数据，所以需要打开连接。
    if(!report_center_.Open())
    {
        LOG_ERROR("Start: open center report error.");
        if(GetDataCenterIp() == false)
        {
            LOG_ERROR("Start: GetDataCenterIp failed.");
        }
    }

    if(!report_block_.Open())
    {
        LOG_ERROR("Start: open block report error.");
        if(GetBlockIp() == false)
        {
            LOG_ERROR("Start: GetBlockIp failed.");
        }
    }

    area_id_ = szAreaId;
    org_id_ = szOrgId;

    if(detect_mode_ == 1)
    {
        StandardScan(ipRange);
    }
    else if(detect_mode_ == 1)
    {
        DepthScan();
    }

    return 0;
}

int DetectHost::DepthScan()
{
    LOG_INFO("DepthScan: begin.");
    return 0;
}

void* RecvRegProc(void* param)
{
    DetectHost* context = static_cast<DetectHost*>(param);
    context->RecvClientPack();
    pthread_exit(NULL);
}

void* RecvPingProc(void* param)
{
    DetectHost* context = static_cast<DetectHost*>(param);
    context->RecvIcmpPack();
    pthread_exit(NULL);
}

void* RecvNbtProc(void* param)
{
    DetectHost* context = static_cast<DetectHost*>(param);
    context->RecvNbtPack();
    pthread_exit(NULL);
}

void DetectHost::RecvClientPack()
{
    int ret;
    fd_set fst;

    while(register_scan_stop_ == 0)
    {
        FD_ZERO(&fst);
        FD_SET(sockcd_, &fst);

        struct timeval tm_out;
        tm_out.tv_sec = 0;
        tm_out.tv_usec = SOCKET_SELECT_TIMEOUT;

        ret = select(sockcd_ + 1, &fst, NULL, NULL, &tm_out);

        if(ret == 0 || ret == -1)
        {
            continue;
        }

        if(FD_ISSET(sockcd_, &fst))
        {
            char recvBuf[2048] = {0};
            struct sockaddr_in addrfrom = {0};
            int size = sizeof(addrfrom);
            ret = recvfrom(sockcd_, recvBuf, 2048, 0, (sockaddr*)&addrfrom, (socklen_t*)&size);

            if(ret > 0 )
            {
                ParseClientPack(ntohl(addrfrom.sin_addr.s_addr), recvBuf, ret);
            }
        }
    }

    //register_mutex_.Lock();
    mapDev::iterator it = register_dev_.begin();
    while(it != register_dev_.end())
    {
        register_dev_keep_.insert(mapDev::value_type(it->first, it->second));
        it++;
    }
    //register_mutex_.Unlock();

    printf("client exit\n");
}

void DetectHost::ParseClientPack(ULONG uip, char* data, int iLen)
{
    struct sockaddr_in Addr = {0};
    Addr.sin_addr.s_addr = htonl(uip);
    std::string szip = inet_ntoa(Addr.sin_addr);

    Json::Value json_object;
    Json::Reader reader;

    std::string szjdata;
    _CEMS_NET_HEAD *pheader = (_CEMS_NET_HEAD * )data;

    std::string szTemp;
    if(pheader)
    {
        szTemp.append( (char*)(data + pheader->wHeadSize) , pheader->dwDataSize);
        szjdata = szTemp;
    }

    if(!reader.parse(szjdata, json_object))
    {
        std::string szErrorText = "解析客户端回包错误";
        szErrorText += szip;
        szErrorText += szjdata;
        printf("%s\n", szErrorText.c_str());
        LOG_ERROR("parseClientPack: parse client package error:"<<szjdata);
        return;
    }

    DEV_INFO devinfo;
    devinfo.szGroupName = CodeConvert::g2u(json_object["groupName"].asString());
    devinfo.szHostName = CodeConvert::g2u(json_object["hostName"].asString());
    devinfo.szIP = CodeConvert::g2u(json_object["ip"].asString());
    devinfo.szMac = CodeConvert::g2u(json_object["mac"].asString());
    devinfo.szDevId = CodeConvert::g2u(json_object["devOnlyId"].asString());
    devinfo.szBoot = "1";
    devinfo.szFireWall = "2";
    devinfo.szAreaId = area_id_;
    devinfo.szOrgId = org_id_;
    devinfo.szRegAreaId = CodeConvert::g2u(json_object["areaId"].asString());
    devinfo.szRegOrgId = CodeConvert::g2u(json_object["orgId"].asString());
    devinfo.count = 0;

    //register_mutex_.Lock();
    register_dev_.insert(mapDev::value_type(uip, devinfo));
    //register_mutex_.Unlock();

    printf("areaId = %s\n", devinfo.szAreaId.c_str());
    printf("regAreaId = %s\n", devinfo.szRegAreaId.c_str());

    if(devinfo.szAreaId.compare(devinfo.szRegAreaId) != 0) //漫游主机(区域id不同)
    {
        mapDev::iterator iter = roaming_dev_.find(uip);
        if(iter == roaming_dev_.end())
        {
            roaming_dev_.insert(mapDev::value_type(uip, devinfo));//漫游设备
            std::string szText = CreateSendText(devinfo);

            printf("漫游设备: %s\n", szText.c_str());
            LOG_DEBUG("parseClientPack: discover roaming device: "<<szText.c_str());

            bool bret;
            bret = report_center_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
            if(!bret)
            {
                printf("send to data service fail\n");
                LOG_ERROR("parseClientPack: send data to center service failed. data:"<<szText);
            }

            bret = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
            if(!bret)
            {
                printf("send to block service fail\n");
                LOG_ERROR("parseClientPack: send data to block service failed. data:"<<szText);
            }
        }
        else
        {
            iter->second.count = 0;
        }
    }
}

std::string DetectHost::CreateSendText(DEV_INFO & info)
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

bool DetectHost::RecvIcmpPack()
{
    int len;
    int nfd  = 0;

    int rfds = sockfd_ + 1;

    struct sockaddr_in from_addr;
    socklen_t fromlen = sizeof(from_addr);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = SOCKET_SELECT_TIMEOUT;

    fd_set rset;

    FD_ZERO(&rset);
    FD_SET(sockfd_, &rset);

    while(!icmp_scan_stop_)
    {
        FD_ZERO(&rset);
        FD_SET(sockfd_, &rset);

        struct timeval temp = timeout;
        if ((nfd = select(rfds, &rset, NULL, NULL, &temp)) == -1) 
        {
            continue;
        }

        if (nfd == 0) 
        {
            continue;
        }

        if (FD_ISSET(sockfd_, &rset)) 
        {
            char recvpacket[PACKET_SIZE] = {0};
            if((len = recvfrom(sockfd_, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from_addr, &fromlen)) < 0)
            {
                continue;
            }

            ULONG uip  = ntohl(from_addr.sin_addr.s_addr);
            std::string ip = inet_ntoa(from_addr.sin_addr);

            if(ParseIcmpPack(recvpacket, len) == false)
            {
                continue;
            }
            else
            {
                if(register_dev_.find(uip) != register_dev_.end())
                {
                    //如果该设备是一个已注册设备，则将该设备从未注册设备中清除。
                    unregister_dev_.erase(uip);
                    unregister_dev_keep_.erase(uip);
                    continue;
                }
                DEV_INFO device;
                device.szIP = ip;
                device.szBoot = "1";
                device.szAreaId = area_id_;
                device.szOrgId  = org_id_;
                device.szRegAreaId = area_id_;
                device.szRegOrgId = org_id_;
                device.szFireWall = "2";
                device.count = 0;

                unregister_dev_.insert(mapDev::value_type(uip, device));

                //如果该设备是一个老的未注册设备，则不进行上报
                //如果该设备是一个新的未注册设备，则进行上报，
                //并且上报成功之后，该设备定义为老的未注册设备，
                //下次扫描到之后不进行上报。
                if(unregister_dev_keep_.find(uip) != unregister_dev_keep_.end())
                {
                    continue;
                }
                else
                {
                    if(device.szMac.empty())
                    {
                        string mac;
                        get_mac_addr(ip, mac);
                        device.szMac = mac;
                    }
                    std::string szText = CreateSendText(device);
                    LOG_DEBUG("recvPingPack: discover unregister device: " << szText.c_str());
                    printf("PING发现未注册主机 = %s\n", szText.c_str());

                    bool bret1, bret2;
                    bret1 = report_center_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
                    if(!bret1)
                    {
                        LOG_ERROR("recvPingPack: send data to center service error, data:"<<szText.c_str());
                        printf("send to data service fail\n");
                    }

                    bret2 = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
                    if(!bret2)
                    {
                        LOG_ERROR("recvPingPack: send data to block service error, data:"<<szText.c_str());
                        printf("send to block service fail\n");
                    }
                    if(bret1 && bret2)
                    {
                        unregister_dev_keep_.insert(mapDev::value_type(uip, device));
                    }
                }
            }
        }
    }
    printf("ping thread exit\n");
    return true;
}

bool DetectHost::ParseIcmpPack(char *buf,int len)
{
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend, tvrecv, tvresult;
    double rtt;

    ip = (struct ip *)buf;
    iphdrlen = ip->ip_hl << 2;    /*求ip报头长度,即ip报头的长度标志乘4*/
    icmp = (struct icmp *)(buf + iphdrlen);  /*越过ip报头,指向ICMP报头*/
    len -= iphdrlen;            /*ICMP报头及ICMP数据报的总长度*/
    if(len < 8)                /*小于ICMP报头长度则不合理*/
    {
        printf("ICMP packets\'s length is less than 8\n");
        return false;
    }
    /*确保所接收的是我所发的的ICMP的回应*/
    if( (icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid_) )
    {
        tvsend=(struct timeval *)icmp->icmp_data;
        gettimeofday(&tvrecv,NULL);  /*记录接收时间*/
        tvresult = tvSub(tvrecv, *tvsend);  /*接收和发送的时间差*/
        rtt=tvresult.tv_sec*1000 + tvresult.tv_usec/1000;  /*以毫秒为单位计算rtt*/

        return true;
    }
    else 
    {
        return false;
    }
}

void DetectHost::RecvNbtPack()
{
    int ret;
    fd_set fst;

    while(nbt_scan_stop_ == 0)
    {
        FD_ZERO(&fst);
        FD_SET(sockud_, &fst);

        struct timeval tm_out;
        tm_out.tv_sec = 0;
        tm_out.tv_usec = SOCKET_SELECT_TIMEOUT;

        if((ret = select(sockud_ + 1, &fst, NULL, NULL, &tm_out)) == -1)
        {
            continue;
        }

        if(ret == 0)
        {
            continue;
        }

        if (FD_ISSET(sockud_, &fst))
        {
            char RecvBuf[1024] = {0};
            struct sockaddr_in addrfrom = {0};
            int size = sizeof(addrfrom);

            if((ret = recvfrom(sockud_, RecvBuf, 1024, 0, (sockaddr*)&addrfrom, (socklen_t*)&size)) < 0)
            {
                continue;
            }

            ULONG ipRev = ntohl(addrfrom.sin_addr.s_addr);

            if(register_dev_.find(ipRev) != register_dev_.end())
            {
                //如果该设备是已注册设备，则从未注册设备中进行清除。
                unregister_dev_.erase(ipRev);
                unregister_dev_keep_.erase(ipRev);
                continue;
            }

            _NBT_INFO  info;
            ParseNbtPack(ipRev, RecvBuf, strlen(RecvBuf), &info);

            DEV_INFO device;
            device.szBoot = "1";
            device.szAreaId = area_id_;
            device.szOrgId  = org_id_;
            device.szRegAreaId = area_id_;
            device.szRegOrgId = org_id_;
            device.szHostName = info.szWorkName;
            device.szGroupName  = info.szGroupName;			
            device.szIP = info.szIp;
            device.szMac = info.szMac;
            device.szFireWall = "2";
            device.count = 0;	

            unregister_dev_.insert(std::pair<ULONG, DEV_INFO>(ipRev, device));

            if(unregister_dev_keep_.find(ipRev) != unregister_dev_keep_.end())
            {
                //如果该设备是未注册设备，并且该设备不是新扫描到的未注册设备，
                //则不进行上报。
                continue;
            }
            else
            {
                std::string szText = CreateSendText(device);
                LOG_DEBUG("recvNbtPack: discover unregister device: " << szText.c_str());
                printf("NBT发现未注册主机 = %s\n", szText.c_str());
                bool bret1, bret2;
                bret1 = report_center_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
                if(!bret1)
                {
                    LOG_ERROR("recvNbtPack: send data to center service error, data:"<<szText.c_str());
                    printf("send to data service fail\n");
                }

                bret2 = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
                if(!bret2)
                {
                    LOG_ERROR("recvNbtPack: send data to block service error, data:"<<szText.c_str());
                    printf("send to block service fail\n");
                }
                if(bret1 && bret2)
                {
                    //如果上报成功，则将该设备定义为老的未注册设备。
                    unregister_dev_keep_.insert(std::pair<ULONG, DEV_INFO>(ipRev, device));
                }
            }
        }
    }	
}

void DetectHost::ParseNbtPack(ULONG uip, char* buffer, ULONG uLen,  _NBT_INFO* pinfo)
{
    std::string szip, szgroup, szhostname, szmac;

    struct sockaddr_in toAddr = {0};
    toAddr.sin_addr.s_addr = htonl(uip);
    szip = inet_ntoa(toAddr.sin_addr);

    USHORT NumberOfNames = 0;
    memcpy(&NumberOfNames, buffer + 56, 1);

    int i = 0;
    char NetbiosName[17] = {0};

    memcpy(NetbiosName, buffer + 57, 15);

    std::string flags;
    for(int ic = 0; ic < NumberOfNames; ic++)
    {
        if(!szhostname.empty() && !szgroup.empty())
        {
            break;
        }

        unsigned char ch;
        memcpy(&ch, buffer + 57 + ic * 18 + 16, 1);

        char NetbiosName[17] = {0};
        memcpy(NetbiosName, buffer + 57 + ic*18, 15);

        unsigned int u = (unsigned int)ch;
        if((u & 0x80) == 0) //unique name
        {
            szhostname = NetbiosName;
        }
        else
        {
            szgroup = NetbiosName;
        }
    }

    USHORT mac[6] = {0};
    for(i = 0; i < 6; i++)
    {
        char buff[10] = {0};
        memcpy(&mac[i], buffer + 57 + NumberOfNames*18 + i, 1);

        if(i != 5)
        {
            sprintf(buff, "%02X-", mac[i]);
        }
        else
        {
            sprintf(buff, "%02X", mac[i]);
        }
        szmac += buff;
    }

    if(szmac.compare("00-00-00-00-00-00") == 0)
    {
        szmac = "";
    }

    pinfo->szIp = szip;
    pinfo->szMac = szmac;
    pinfo->szWorkName = szhostname;
    pinfo->szGroupName = szgroup;
}

void* SendPackProc(void* param)
{
    DetectHost* context = ((SendPackParam*)param)->context;
    context->SendDetectPack(((SendPackParam*)param)->ip_range, ((SendPackParam*)param)->send_type);
    pthread_exit(NULL);
}

int DetectHost::SendDetectPack(MAP_COMMON* ipRange, SEND_TYPE send_type)
{
    MAP_COMMON::iterator iter = ipRange->begin();

    while(iter != ipRange->end())
    {
        SendPacks((char*)iter->first.c_str(), (char*)iter->second.c_str(), send_type);
        iter ++;
    }

    usleep(THREAD_SLEEP_TIME);
    if(send_type == s_reg)
    {
        register_scan_stop_ = 1;
    }
    else if(send_type == s_ping)
    {
        icmp_scan_stop_ = 1;
    }
    else if(send_type == s_nbt)
    {
        nbt_scan_stop_ = 1;
    }

    return 0;
}

int DetectHost::SendPacks( char* start, char* end, SEND_TYPE type)
{
    usleep(50*1000);
    int ret = 0;
    ULONG first = ntohl(inet_addr(start));
    ULONG second = ntohl(inet_addr(end));

    ULONG uip = first;
    while(uip < second + 1)
    {
        SendPack(uip, type);	
        uip ++;
        usleep(10*1000);
    }

    return ret;
}

bool DetectHost::SendPack(unsigned long uip, SEND_TYPE type)
{
    static int i = 0;
    char sendpacket[PACKET_SIZE] = {0};

    struct in_addr addr;
    addr.s_addr = htonl(uip);

    struct sockaddr_in dest_addr;

    if (!getsockaddr(inet_ntoa(addr), &dest_addr)) 
    {
        return false;
    }

    int len = 0;

    if(type == s_nbt)
    {
        struct Q_NETBIOSNS nbns;
        MakeQueryPack(nbns);

        struct sockaddr_in toAddr = {0};

        toAddr.sin_family = AF_INET;
        toAddr.sin_addr.s_addr = htonl(uip);
        toAddr.sin_port = htons(NBNS_PORT);

        if((len = sendto(sockud_,  (const char*)&nbns, sizeof(nbns), 0, (struct sockaddr *)&toAddr, sizeof(toAddr))) < 0 )
        {
            printf("udp sendto fail, size = %d, pack size = %lu\n", len, sizeof(nbns));
            return false;
        }
    }

    if(type == s_ping)
    {
        int packetsize = MakeIcmpPack(i++, (struct icmp*)sendpacket); //设置ICMP报头

        if((len = sendto(sockfd_, sendpacket, packetsize, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr))) < 0  )
        {
            printf("icmp sendto fail\n");
            return false;
        }
    }   

    if(type == s_reg)
    {
        char sendbuffer[1024] = {0};
        int  len = 1024;

        MakeClientPack(sendbuffer, len);

        struct sockaddr_in toAddr = {0};

        toAddr.sin_family = AF_INET;
        toAddr.sin_addr.s_addr = htonl(uip);
        toAddr.sin_port = htons(CLIENT_PORT);

        if((len = sendto(sockcd_, sendbuffer, len, 0, (struct sockaddr *)&toAddr, sizeof(toAddr))) < 0)
        {
            printf("client udp sendto fail, size = %d\n", len);
            return false;		
        }		
    }

    return true;
}

void DetectHost::MakeClientPack(char* sendBuffer, int & len)
{
    std::string szFlag = "Server";
    ULONG dwSize = szFlag.length();

    _CEMS_NET_HEAD header = {0};

    header.dwFlag = *(unsigned int *)"pde_";
    header.dwVersion = (1);	
    header.dwDataSize = dwSize;
    header.dwCrc = atoi(calCRC(szFlag).c_str());

    header.dwMaxCode = UDP_MAXCODE; //(0x00010100);
    header.dwMinCode = UDP_MINCODE; //(2);
    header.dwMsgCode = (1);
    //header.szSessionId = {0};

    header.wHeadSize = sizeof(header);
    header.wType = 0;
    header.wCount = 0;
    header.wIndex = 0;

    char * pBuffer = new char[sizeof(header) + dwSize];

    memcpy(pBuffer, &header, sizeof(header));
    memcpy(pBuffer + sizeof(header), szFlag.c_str(), dwSize);

    len = (sizeof(header) + dwSize);

    memcpy(sendBuffer, pBuffer, len);
    delete []pBuffer;	
}

int DetectHost::MakeIcmpPack(int pack_no, struct icmp* icmp)
{
    int packsize;
    struct icmp *picmp;
    struct timeval *tval;

    picmp = icmp;
    picmp->icmp_type=ICMP_ECHO;
    picmp->icmp_code=0;
    picmp->icmp_cksum=0;
    picmp->icmp_seq=pack_no;
    picmp->icmp_id= pid_;
    packsize= 8 + g_datalen;
    tval= (struct timeval *)icmp->icmp_data;
    gettimeofday(tval,NULL);    /*记录发送时间*/
    picmp->icmp_cksum=getChksum((unsigned short *)icmp,packsize); /*校验算法*/
    return packsize;
}

void DetectHost::MakeQueryPack(struct Q_NETBIOSNS& nbns)
{
    nbns.tid = 0x0000;
    nbns.flags = 0x0000;
    nbns.questions = 0x0100;
    nbns.answerRRS = 0x0000;
    nbns.authorityRRS = 0x0000;
    nbns.additionalRRS = 0x0000;
    nbns.name[0] = 0x20;
    nbns.name[1] = 0x43;
    nbns.name[2] = 0x4b;

    int j = 0;
    for(j = 3; j < 34; j++)
        nbns.name[j] = 0x41;

    nbns.name[33] = 0x00;
    nbns.type = 0x2100;
    nbns.classe = 0x0100;
}

int DetectHost::ClientProbe(MAP_COMMON* ipRange)
{
    LOG_INFO("ClientProbe: start.");
    if((sockcd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("socket udp create fail\n");
        return -1;
    }

    pthread_t t_send;
    pthread_t t_recv;
    SendPackParam param;
    param.context = this;
    param.ip_range = ipRange;
    param.send_type = s_reg;

    register_scan_stop_ = 0;
    int rc;
    rc = pthread_create(&t_send, NULL, SendPackProc, (void*)&param);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }

    rc = pthread_create(&t_recv, NULL, RecvRegProc, (void*)this);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }

    rc = pthread_join(t_send, NULL);
    if(rc != 0)
    {
        printf("pthread_join error = %s\n", strerror(rc));
    }

    rc = pthread_join(t_recv, NULL);
    if(rc != 0)
    {
        printf("pthread_join error = %s\n", strerror(rc));
    }

    close(sockcd_);
    LOG_INFO("ClientProbe: end.");
    return 0;
}

int DetectHost::IcmpProbe(MAP_COMMON* const ipRange)
{
    LOG_INFO("IcmpProbe: begin icmp probe.");
    pid_ = getpid();
    struct protoent *protocol;
    if((protocol = getprotobyname("icmp")) == NULL)
    {
        perror("getprotobyname");
        return -1;
    }

    if((sockfd_ = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
    {
        printf("socket raw  creat fail\n");
        return -1;
    }

    int size = 200 * 1024;
    int m_recvTimeout = 5*1000; //ms

    /*扩大套接字接收缓冲区到50K这样做主要为了减小接收缓冲区溢出的
      的可能性,若无意中ping一个广播地址或多播地址,将会引来大量应答*/
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size) );

    /*设置接收超时*/
    int timeout = m_recvTimeout;
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    /*设置非阻塞*/
    /*int flags = fcntl(sockfd_, F_GETFL, 0);
      fcntl(sockfd_, F_SETFL, flags | O_NONBLOCK);*/

    pthread_t t_sendDetec;
    pthread_t t_recvPing;

    SendPackParam param;
    param.context = this;
    param.ip_range = ipRange;
    param.send_type = s_ping;

    icmp_scan_stop_ = 0;

    int rc = pthread_create(&t_sendDetec, NULL, SendPackProc, (void*)&param);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }
    rc = pthread_create(&t_recvPing, NULL, RecvPingProc, (void*)this);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }
    rc = pthread_join(t_sendDetec, NULL);
    if(rc != 0)
    {
        printf("pthread_join error = %s\n", strerror(rc));
    }
    rc = pthread_join(t_recvPing, NULL);
    if(rc != 0)
    {
        printf("pthread_join error = %s\n", strerror(rc));
    }

    close(sockfd_);
    LOG_INFO("IcmpProbe: icmp probe end.");
    return 0;
}

int DetectHost::NbtProbe(MAP_COMMON* const ipRange)
{
    LOG_INFO("NbtProbe: start.");
    nbt_scan_stop_ = 0;
    if((sockud_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("socket udp create fail\n");
        return -1;
    }
 
    int rc;
    pthread_t t_sendDetec;
    pthread_t t_recvNbt;

    SendPackParam param;
    param.context = this;
    param.send_type = s_nbt;
    param.ip_range = ipRange;

    rc = pthread_create(&t_sendDetec, NULL, SendPackProc, (void*)&param);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }
    rc = pthread_create(&t_recvNbt, NULL, RecvNbtProc, (void*)this);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }
    rc = pthread_join(t_sendDetec, NULL);
    if(rc != 0)
    {
        printf("pthread_join error = %s\n", strerror(rc));
    }

    rc = pthread_join(t_recvNbt, NULL);
    if(rc != 0)
    {
        printf("pthread_join error = %s\n", strerror(rc));
    }
    close(sockud_);
    LOG_INFO("NbtProbe: end.");
    return 0;
}

int DetectHost::StandardScan(MAP_COMMON * ipRange)
{
    LOG_INFO("start standerd scan.");
    ClientProbe(ipRange);
    NbtProbe(ipRange);
    IcmpProbe(ipRange);
    LOG_INFO("standerd scan end.");
    return 0;
}

int DetectHost::CalculateCloseDev()
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

            struct sockaddr_in Addr = {0};
            Addr.sin_addr.s_addr = htonl(it->first);
            std::string szip = inet_ntoa(Addr.sin_addr);

            printf("发现关机设备 = %s\n", szip.c_str());
            LOG_DEBUG("DetectClose: discover register->shutdown device: "<<szText.c_str());

            bool bret;
            bret = report_center_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to center service error, data:"<<szText.c_str());
                printf("send to data service fail\n");
            }

            bret = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to block service error, data:"<<szText.c_str());
                printf("send to block service fail\n");

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

            struct sockaddr_in Addr = {0};
            Addr.sin_addr.s_addr = htonl(it->first);
            std::string szip = inet_ntoa(Addr.sin_addr);

            printf("发现关机设备 = %s\n", szip.c_str());
            LOG_DEBUG("DetectClose: discover unregister->shutdown device: "<<szText.c_str());

            bool bret;
            bret = report_center_.SendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to center service error, data:"<<szText.c_str());
                printf("send to data service fail\n");
            }

            bret = report_block_.SendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to block service error, data:"<<szText.c_str());
                printf("send to block service fail\n");

            } 
            unregister_dev_keep_.erase(it++);
            continue;
        }
        ++it;
    }

    mapDev ().swap(register_dev_);
    mapDev ().swap(unregister_dev_);
    report_block_.Close();
    report_center_.Close();
    return 0;
}

struct timeval tvSub(struct timeval timeval1,struct timeval timeval2)
{
    struct timeval result;
    result = timeval1;
    if (result.tv_usec < timeval2.tv_usec && timeval2.tv_usec < 0)
    {
        --result.tv_sec;
        result.tv_usec += 1000000;
    }
    result.tv_sec -= timeval2.tv_sec;
    return result;
}

bool getsockaddr(const char * hostOrIp, struct sockaddr_in* sockaddr) 
{
    struct hostent *host;
    struct sockaddr_in dest_addr;
    unsigned long inaddr=0l;
    bzero(&dest_addr,sizeof(dest_addr));
    dest_addr.sin_family=AF_INET;
    /*判断是主机名还是ip地址*/
    if( (inaddr = inet_addr(hostOrIp)) == INADDR_NONE)
    {
        if((host = gethostbyname(hostOrIp)) == NULL) /*是主机名*/
        {
            /*printf("gethostbyname error:%s\n", hostOrIp);*/
            return false;
        }
        memcpy( (char *)&dest_addr.sin_addr,host->h_addr,host->h_length);
    }
    /*是ip地址*/
    else if (!inet_aton(hostOrIp, &dest_addr.sin_addr))
    {
        /*memcpy( (char *)&dest_addr,(char *)&inaddr,host->h_length);
          fprintf(stderr, "unknow host:%s\n", hostOrIp);*/
        return false;
    }
    *sockaddr = dest_addr;
    return true;
}

/*校验和算法*/
unsigned short getChksum(unsigned short *addr,int len)
{
    int nleft=len;
    int sum=0;
    unsigned short *w=addr;
    unsigned short answer=0;

    /*把ICMP报头二进制数据以2字节为单位累加起来*/
    while(nleft>1)
    {
        sum+=*w++;
        nleft-=2;
    }
    /*若ICMP报头为奇数个字节，会剩下最后一字节。把最后一个字节视为一个2字节数据的高字节，这个2字节数据的低字节为0，继续累加*/
    if( nleft==1)
    {
        *(unsigned char *)(&answer)=*(unsigned char *)w;
        sum+=answer;
    }
    sum=(sum>>16)+(sum&0xffff);
    sum+=(sum>>16);
    answer=~sum;
    return answer;
}
