#include "ConfigService.h"
#include "upreport.h"
#include "service_reg.h"
#include "DetectHost.h"
#include "log.h"
#include "common_function.h"
#include "json/json.h"
#include "conv.h"
#include "arp.h"

#include <netinet/ip_icmp.h>

using namespace cems::service::scan;

#define PACKET_SIZE     4096
#define MAX_WAIT_TIME   5
#define MAX_NO_PACKETS  3

#define THREAD_SLEEP_TIME	5*1000*1000
#define SOCKET_SELECT_TIMEOUT	3*1000*1000

#define SCAN_UNREPLY_COUNT	3

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

CUpReport report_block;
CUpReport report_center;

mapDev	  g_mapUnRegKeep;	 
mapDev    g_mapUnRegist; 

mapDev	  g_mapRegKeep;
mapDev    g_mapRegist;

mapDev	  g_mapPing;
mapDev	  g_mapRoaming;

pid_t   g_pid;
int     g_datalen = 40;

int     m_sockfd;
int     m_sockud;
int     m_sockcd;

volatile  int  g_bRegScanStop = 0;
volatile  int  g_bUnRegScanStop = 0;

std::string g_szAreaId;
std::string g_szOrgId;

pthread_mutex_t g_mutex_scan = PTHREAD_MUTEX_INITIALIZER;

void  	MakeClientPack(char* sendBuffer, int & len);
void  	MakeQueryPack(struct Q_NETBIOSNS& nbns);
void  	parseClientPack(ULONG uip,  char* data, int iLen);
void  	parsePack(ULONG uip, char* buffer, ULONG uLen,  _NBT_INFO* pinfo);

std::string  CreateSendText(DEV_INFO & info);

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

/*设置ICMP报头*/
int packIcmp(int pack_no, struct icmp* icmp)
{
    int packsize;
    struct icmp *picmp;
    struct timeval *tval;

    picmp = icmp;
    picmp->icmp_type=ICMP_ECHO;
    picmp->icmp_code=0;
    picmp->icmp_cksum=0;
    picmp->icmp_seq=pack_no;
    picmp->icmp_id= g_pid;
    packsize= 8 + g_datalen;
    tval= (struct timeval *)icmp->icmp_data;
    gettimeofday(tval,NULL);    /*记录发送时间*/
    picmp->icmp_cksum=getChksum((unsigned short *)icmp,packsize); /*校验算法*/
    return packsize;
}

/*剥去ICMP报头*/
bool unpackIcmp(char *buf,int len)
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
    if( (icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == g_pid) )
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

bool getsockaddr(const char * hostOrIp, struct sockaddr_in* sockaddr) {
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
    else if (!inet_aton(hostOrIp, &dest_addr.sin_addr)){
        /*memcpy( (char *)&dest_addr,(char *)&inaddr,host->h_length);
          fprintf(stderr, "unknow host:%s\n", hostOrIp);*/
        return false;
    }
    *sockaddr = dest_addr;
    return true;
}

/*发送ICMP报文*/
bool sendPack(unsigned long uip, SEND_TYPE type)
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

        if((len = sendto(m_sockud,  (const char*)&nbns, sizeof(nbns), 0, (struct sockaddr *)&toAddr, sizeof(toAddr))) < 0 )
        {
            printf("udp sendto fail, size = %d, pack size = %lu\n", len, sizeof(nbns));
            return false;
        }
    }

    if(type == s_ping)
    {
        int packetsize = packIcmp(i++, (struct icmp*)sendpacket); //设置ICMP报头

        if((len = sendto(m_sockfd, sendpacket, packetsize, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr))) < 0  )
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

        if((len = sendto(m_sockcd, sendbuffer, len, 0, (struct sockaddr *)&toAddr, sizeof(toAddr))) < 0)
        {
            printf("client udp sendto fail, size = %d\n", len);
            return false;		
        }		
    }

    return true;
}

/*接收所有ICMP报文*/
bool recvPingPack(int sock)
{
    int len;
    int nfd  = 0;

    int rfds = sock + 1;

    struct sockaddr_in from_addr;
    socklen_t fromlen = sizeof(from_addr);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = SOCKET_SELECT_TIMEOUT;

    fd_set rset;

    FD_ZERO(&rset);
    FD_SET(sock, &rset);

    g_mapPing.clear();

    while(!g_bUnRegScanStop)
    {
        FD_ZERO(&rset);
        FD_SET(sock, &rset);

        struct timeval temp = timeout;
        if ((nfd = select(rfds, &rset, NULL, NULL, &temp)) == -1) 
        {
            continue;
        }

        if (nfd == 0) 
        {
            continue;
        }

        if (FD_ISSET(sock, &rset)) 
        {
            char recvpacket[PACKET_SIZE] = {0};
            if((len = recvfrom(sock, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from_addr, &fromlen)) < 0)
            {
                continue;
            }

            ULONG uip  = ntohl(from_addr.sin_addr.s_addr);
            std::string ip = inet_ntoa(from_addr.sin_addr);

            if(unpackIcmp(recvpacket, len) == false)
            {
                continue;
            }
            else
            {
                if(g_mapRegKeep.find(uip) != g_mapRegKeep.end())
                {
                    mapDev::iterator it = g_mapUnRegist.find(uip);
                    if(it != g_mapUnRegist.end())
                    {
                        g_mapUnRegist.erase(it);
                    }

                    it = g_mapUnRegKeep.find(uip);
                    if(it != g_mapUnRegKeep.end())
                    {
                        g_mapUnRegKeep.erase(it);
                    }
                    continue;
                }

                pthread_mutex_lock(&g_mutex_scan);
                if(g_mapUnRegist.find(uip) == g_mapUnRegist.end())
                {
                    DEV_INFO device;
                    device.szIP = ip;
                    device.szBoot = "1";
                    device.szAreaId = g_szAreaId;
                    device.szOrgId  = g_szOrgId;
                    device.szRegAreaId = g_szAreaId;
                    device.szRegOrgId = g_szOrgId;
                    device.szFireWall = "2";
                    device.count = 0;
                    string mac;
                    get_mac_addr(ip, mac);
                    device.szMac = mac;

                    g_mapPing.insert(mapDev::value_type(uip, device));

                }
                pthread_mutex_unlock(&g_mutex_scan);
            }
        }
    }

    printf("ping thread exit\n");

    return true;
}

//接受所有nbt包
void recvNbtPack(int sock)
{
    int ret;
    fd_set fst;

    int count = 0;

    while(!g_bUnRegScanStop)
    {
        FD_ZERO(&fst);
        FD_SET(sock, &fst);

        struct timeval tm_out;
        tm_out.tv_sec = 0;
        tm_out.tv_usec = SOCKET_SELECT_TIMEOUT;

        if((ret = select(sock + 1, &fst, NULL, NULL, &tm_out)) == -1)
        {
            continue;
        }

        if(ret == 0)
        {
            continue;
        }

        if (FD_ISSET(sock, &fst))
        {
            char RecvBuf[1024] = {0};
            struct sockaddr_in addrfrom = {0};
            int size = sizeof(addrfrom);

            if((ret = recvfrom(sock, RecvBuf, 1024, 0, (sockaddr*)&addrfrom, (socklen_t*)&size)) < 0)
            {
                continue;
            }

            ULONG ipRev = ntohl(addrfrom.sin_addr.s_addr);

            if(g_mapRegKeep.find(ipRev) != g_mapRegKeep.end())
            {
                mapDev::iterator it = g_mapUnRegist.find(ipRev);
                if(it != g_mapUnRegist.end())
                {
                    g_mapUnRegist.erase(it);
                }

                it = g_mapUnRegKeep.find(ipRev);
                if(it != g_mapUnRegKeep.end())
                {
                    g_mapUnRegKeep.erase(it);
                }
                continue;
            }

            _NBT_INFO  info;
            parsePack(ipRev, RecvBuf, strlen(RecvBuf), &info);

            DEV_INFO device;
            device.szBoot = "1";
            device.szAreaId = g_szAreaId;
            device.szOrgId  = g_szOrgId;
            device.szRegAreaId = g_szAreaId;
            device.szRegOrgId = g_szOrgId;
            device.szHostName = info.szWorkName;
            device.szGroupName  = info.szGroupName;			
            device.szIP = info.szIp;
            device.szMac = info.szMac;
            device.szFireWall = "2";
            device.count = 0;	

            pthread_mutex_lock(&g_mutex_scan);
            mapDev::iterator iter = g_mapUnRegist.find(ipRev);
            if(iter == g_mapUnRegist.end())
            {
                g_mapUnRegist.insert(std::pair<ULONG, DEV_INFO>(ipRev, device));

                count = g_mapUnRegist.size();			

                std::string szText = CreateSendText(device);	

                if(g_mapUnRegKeep.find(ipRev) == g_mapUnRegKeep.end())
                {
                    /*char buffer[256] = {0};
                    sprintf(buffer, "NBT发现未注册主机 = %s", szText.c_str());
                    log.WriteLog(buffer, LOG_INFO);*/
                    LOG_DEBUG("recvNbtPack: discover unregister device: " << szText.c_str());

                    printf("NBT发现未注册主机 = %s\n", szText.c_str());
                    bool bret;
                    bret = report_center.sendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
                    if(!bret)
                    {
                        LOG_ERROR("recvNbtPack: send data to center service error, data:"<<szText.c_str());
                        printf("send to data service fail\n");
                    }

                    bret = report_block.sendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
                    if(!bret)
                    {
                        LOG_ERROR("recvNbtPack: send data to block service error, data:"<<szText.c_str());
                        printf("send to block service fail\n");
                    }
                }

            }
            else
            {
                iter->second = device;					
            }
            pthread_mutex_unlock(&g_mutex_scan);

        }
    }	

    mapDev::iterator iter = g_mapPing.begin();
    while(iter != g_mapPing.end())
    {
        if(g_mapUnRegist.find(iter->first) == g_mapUnRegist.end())
        {
            g_mapUnRegist.insert(mapDev::value_type(iter->first, iter->second));

            std::string szText = CreateSendText(iter->second);

            if(g_mapUnRegKeep.find(iter->first) == g_mapUnRegKeep.end())
            {
                /*char buffer[256] = {0};
                sprintf(buffer, "PING发现未注册主机 = %s", szText.c_str());
                log.WriteLog(buffer, LOG_INFO);*/
                LOG_DEBUG("recvPingPack: discover unregister device: " << szText.c_str());

                printf("PING发现未注册主机 = %s\n", szText.c_str());

                bool bret;
                bret = report_center.sendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
                if(!bret)
                {
                    LOG_ERROR("recvPingPack: send data to center service error, data:"<<szText.c_str());
                    printf("send to data service fail\n");
                }

                bret = report_block.sendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
                if(!bret)
                {
                    LOG_ERROR("recvPingPack: send data to block service error, data:"<<szText.c_str());
                    printf("send to block service fail\n");
                }
            }
        }
        iter++;
    }

    g_mapPing.clear();	

    printf("nbt thread exit\n");

}

std::string  CreateSendText(DEV_INFO & info)
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

void MakeClientPack(char* sendBuffer, int & len)
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

void  parseClientPack(ULONG uip,  char* data, int iLen)
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
    devinfo.szAreaId = g_szAreaId;
    devinfo.szOrgId = g_szOrgId;
    devinfo.szRegAreaId = CodeConvert::g2u(json_object["areaId"].asString());
    devinfo.szRegOrgId = CodeConvert::g2u(json_object["orgId"].asString());
    devinfo.count = 0;

    g_mapRegist.insert(mapDev::value_type(uip, devinfo));

    printf("areaId = %s\n", devinfo.szAreaId.c_str());
    printf("regAreaId = %s\n", devinfo.szRegAreaId.c_str());

    if(devinfo.szAreaId.compare(devinfo.szRegAreaId) != 0) //漫游主机(区域id不同)
    {
        mapDev::iterator iter = g_mapRoaming.find(uip);
        if(iter  == g_mapRoaming.end())
        {
            g_mapRoaming.insert(mapDev::value_type(uip, devinfo));//漫游设备

            std::string szText = CreateSendText(devinfo);

            printf("漫游设备: %s\n", szText.c_str());
            LOG_DEBUG("parseClientPack: discover roaming device: "<<szText.c_str());

            bool bret;
            bret = report_center.sendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
            if(!bret)
            {
                printf("send to data service fail\n");
                LOG_ERROR("parseClientPack: send data to center service failed. data:"<<szText);
            }

            bret = report_block.sendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
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

//接受注册客户端udp回包
void recvClientPack(int sock)
{
    int ret;
    fd_set fst;

    while(!g_bRegScanStop)
    {
        FD_ZERO(&fst);
        FD_SET(sock, &fst);

        struct timeval tm_out;
        tm_out.tv_sec = 0;
        tm_out.tv_usec = SOCKET_SELECT_TIMEOUT;

        ret = select(sock + 1, &fst, NULL, NULL, &tm_out);

        if(ret == 0 || ret == -1)
        {
            continue;
        }

        if(FD_ISSET(sock, &fst))
        {
            char recvBuf[2048] = {0};
            struct sockaddr_in addrfrom = {0};
            int size = sizeof(addrfrom);
            ret = recvfrom(sock, recvBuf, 2048, 0, (sockaddr*)&addrfrom, (socklen_t*)&size);

            if(ret > 0 )
            {
                parseClientPack(ntohl(addrfrom.sin_addr.s_addr), recvBuf, ret);
            }
        }
    }

    mapDev::iterator it = g_mapRegist.begin();
    while(it != g_mapRegist.end())
    {
        struct sockaddr_in Addr = {0};
        Addr.sin_addr.s_addr = htonl(it->first);
        std::string szip = inet_ntoa(Addr.sin_addr);

        g_mapRegKeep.insert(mapDev::value_type(it->first, it->second));

        it++;
    }

    printf("client exit\n");

}

int SendPacks( char* start, char* end, SEND_TYPE type)
{
    usleep(50*1000);
    int ret = 0;
    ULONG first = ntohl(inet_addr(start));
    ULONG second = ntohl(inet_addr(end));

    ULONG uip = first;
    while(uip < second + 1)
    {
        sendPack(uip, type);	
        uip ++;
        usleep(10*1000);
    }

    return ret;
}

void* SendDetecProc(void* param)
{
    MAP_COMMON * ipRange = (MAP_COMMON*)param;
    MAP_COMMON::iterator iter = ipRange->begin();

    while(iter != ipRange->end())
    {
        SendPacks((char*)iter->first.c_str(), (char*)iter->second.c_str(),  s_ping);
        SendPacks((char*)iter->first.c_str(), (char*)iter->second.c_str(),  s_nbt);
        SendPacks((char*)iter->first.c_str(), (char*)iter->second.c_str(),  s_reg);

        iter ++;
    }

    usleep(THREAD_SLEEP_TIME);

    g_bUnRegScanStop = 1;
    g_bRegScanStop = 1;

    return (void*)NULL;
}

void* SendRegProc(void* param)
{
    g_bRegScanStop = 0;

    MAP_COMMON * ipRange = (MAP_COMMON*)param;
    MAP_COMMON::iterator iter = ipRange->begin();

    while(iter != ipRange->end())
    {
        SendPacks((char*)iter->first.c_str(), (char*)iter->second.c_str(), s_reg);
        iter ++;
    }

    usleep(THREAD_SLEEP_TIME);
    g_bRegScanStop = 1;

    return (void*)NULL;
}

void* RecvPingProc(void* param)
{
    recvPingPack(m_sockfd);
    return (void*)NULL;
}

void* RecvNbtProc(void* param)
{
    recvNbtPack(m_sockud);
    return (void*)NULL;
}

void* RecvRegProc(void* param)
{
    recvClientPack(m_sockcd);	
    return (void*)NULL;
}

void MakeQueryPack(struct Q_NETBIOSNS& nbns)
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

void  parsePack(ULONG uip, char* buffer, ULONG uLen,  _NBT_INFO* pinfo)
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

int DetectRegist(MAP_COMMON * ipRange)
{
    g_bRegScanStop = 0;	

    if((m_sockcd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("socket udp create fail\n");
    }

    pthread_t t_send;
    pthread_t t_recv;

    int rc;
    rc = pthread_create(&t_send, NULL, SendRegProc, (void*)ipRange);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }

    rc = pthread_create(&t_recv, NULL, RecvRegProc, NULL);
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

    close(m_sockcd);

    return 0;
}

int DetectUnRegist(MAP_COMMON * ipRange, std::string szAreaId, std::string szOrgId)
{
    g_szAreaId = szAreaId;
    g_szOrgId = szOrgId;

    //if(g_mapRegKeep.empty()) 		//为空，先进行一次注册扫描
    //{
        DetectRegist(ipRange);
    //}

    g_bUnRegScanStop = 0;
    g_bRegScanStop = 0;

    struct protoent *protocol;

    int size = 200 * 1024;
    int m_recvTimeout = 5*1000; //ms

    g_pid = getpid();

    if((protocol = getprotobyname("icmp")) == NULL)
    {
        perror("getprotobyname");
    }

    if((m_sockfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
    {
        printf("socket raw  creat fail\n");
    }

    if((m_sockud = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("socket udp create fail\n");
    }

    if((m_sockcd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("socket udp create fail\n");
    }

    /*扩大套接字接收缓冲区到50K这样做主要为了减小接收缓冲区溢出的
      的可能性,若无意中ping一个广播地址或多播地址,将会引来大量应答*/
    setsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size) );

    /*设置接收超时*/
    int timeout = m_recvTimeout;
    setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    /*设置非阻塞*/
    /*int flags = fcntl(m_sockfd, F_GETFL, 0);
      fcntl(m_sockfd, F_SETFL, flags | O_NONBLOCK);*/

    int rc;
    pthread_t t_sendDetec;
    pthread_t t_recvNbt;
    pthread_t t_recvPing;
    pthread_t t_recv;

    rc = pthread_create(&t_sendDetec, NULL, SendDetecProc, (void*)ipRange);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }

    rc = pthread_create(&t_recvNbt, NULL, RecvNbtProc, NULL);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }

    rc = pthread_create(&t_recvPing, NULL, RecvPingProc, NULL);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }

    rc = pthread_create(&t_recv, NULL, RecvRegProc, NULL);
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

    rc = pthread_join(t_recvPing, NULL);
    if(rc != 0)
    {
        printf("pthread_join error = %s\n", strerror(rc));
    }

    rc = pthread_join(t_recv, NULL);
    if(rc != 0)
    {
        printf("pthread_join error = %s\n", strerror(rc));
    }

    close(m_sockfd);
    close(m_sockud);
    close(m_sockcd);

    printf("host count = %lu\n", g_mapUnRegist.size());

    return 0;
}

int DetectChange()
{
    g_mapRegKeep.clear();
    g_mapUnRegKeep.clear();
    g_mapRoaming.clear();

    return 1;
}

int DetectInit()
{
    LOG_INFO("DetectInit: begin.");
    g_mapRegist.clear();
    g_mapUnRegist.clear();

    ServiceReg cm;

    //char buffer[1024] = {0};

    std::string szCenterIp, szBlockIp;
    std::string szCenterPort, szBlockPort;
    std::string szCenterOrgId, szBlockOrgId;

    if(cm.Fetch(SERVICE_CODE_CENTER, szCenterOrgId, szCenterIp, szCenterPort))
    {
        /*sprintf(buffer, "fetch data service ip = %s, port = %s", szCenterIp.c_str(), szCenterPort.c_str());
        log.WriteLog(buffer, LOG_INFO);*/

        printf("fetch data service ip = %s, port = %s\n", szCenterIp.c_str(), szCenterPort.c_str());
        LOG_DEBUG("DetectInit: fetch data service ip=" << szCenterIp << ", port=" << szCenterPort);
    }
    else
    {
        LOG_ERROR("DetectInit: fetch data service error.");
    }

    if(cm.Fetch(SERVICE_CODE_BLOCK, szBlockOrgId, szBlockIp, szBlockPort))
    {
        /*sprintf(buffer, "fetch block service ip = %s, port = %s", szBlockIp.c_str(), szBlockPort.c_str());
        log.WriteLog(buffer, LOG_INFO);

        printf("fetch block service ip = %s, port = %s\n", szBlockIp.c_str(), szBlockPort.c_str());*/
        LOG_DEBUG("DetectInit: fetch block service ip=" << szBlockIp << ", port=" << szBlockPort);
    }
    else
    {
        LOG_ERROR("DetectInit: fetch block service error.");
    }

    //szBlockPort = "10400";
    //szCenterPort = "9000";

    //szBlockIp = "192.168.32.6";
    /*szCenterIp = "192.168.0.132"; */

    report_center.init(szCenterIp, atoi(szCenterPort.c_str()));
    report_block.init(szBlockIp, atoi(szBlockPort.c_str()));

    /*if(!report_center.open())
      {
      return -1;
      }

      if(!report_block.open())
      {
      return -1;
      }*/
    LOG_INFO("DetectInit: end.");
    return 	1;
}

int DetectClose()
{
    g_mapRegKeep.swap(g_mapRegist);
    g_mapRegist.clear();

    printf("regist\n");
    mapDev::iterator iter1 = g_mapRegKeep.begin();
    while(iter1 != g_mapRegKeep.end())
    {
        struct sockaddr_in Addr = {0};
        Addr.sin_addr.s_addr = htonl(iter1->first);
        std::string szip = inet_ntoa(Addr.sin_addr);

        printf("keep设备 = %s\n", szip.c_str());
        iter1++;
    }

    printf("unregist\n");
    mapDev::iterator iter2 = g_mapUnRegist.begin();
    while(iter2 != g_mapUnRegist.end())
    {
        struct sockaddr_in Addr = {0};
        Addr.sin_addr.s_addr = htonl(iter2->first);
        std::string szip = inet_ntoa(Addr.sin_addr);

        printf("unreg设备 = %s\n", szip.c_str());

        iter2++;
    }
    printf("roming\n");
    mapDev::iterator iter3 = g_mapRoaming.begin();
    while(iter3 != g_mapRoaming.end())
    {
        mapDev::iterator it = g_mapRegKeep.find(iter3->first);
        if(it != g_mapRegKeep.end())
        {
            iter3->second.count = 0;
        }
        else
        {
            iter3->second.count ++;
            if(iter3->second.count < SCAN_UNREPLY_COUNT)
            {
                iter3++;
                continue;
            }

            iter3->second.szBoot = "0";
            std::string szText = CreateSendText(iter3->second);

            struct sockaddr_in Addr = {0};
            Addr.sin_addr.s_addr = htonl(iter3->first);
            std::string szip = inet_ntoa(Addr.sin_addr);

            printf("发现漫游关机设备 = %s\n", szip.c_str());
            LOG_DEBUG("DetectClose: discover roaming&shutdown device: "<<szText.c_str());

            bool bret;
            bret = report_center.sendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to center service error, data:"<<szText.c_str());
                printf("send to data service fail\n");
            }

            bret = report_block.sendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to block service error, data:"<<szText.c_str());
                printf("send to block service fail\n");
            }

            mapDev::iterator itFind = g_mapRegKeep.find(iter3->first);
            if(itFind != g_mapRegKeep.end())
            {
                g_mapRegKeep.erase(itFind);
            }
            g_mapRoaming.erase(iter3++);

            continue;	
        }
        iter3++;
    }

    mapDev::iterator iter = g_mapUnRegKeep.begin();
    while(iter != g_mapUnRegKeep.end())
    {
        if(g_mapUnRegist.find(iter->first) == g_mapUnRegist.end())
        {
            iter->second.count ++;
            g_mapUnRegist.insert(mapDev::value_type(iter->first, iter->second));

            if(iter->second.count < SCAN_UNREPLY_COUNT)
            {
                iter ++;
                continue;
            }

            iter->second.szBoot = "0";
            std::string szText = CreateSendText(iter->second);

            struct sockaddr_in Addr = {0};
            Addr.sin_addr.s_addr = htonl(iter->first);
            std::string szip = inet_ntoa(Addr.sin_addr);

            printf("发现关机设备 = %s\n", szip.c_str());
            LOG_DEBUG("DetectClose: discover shutdown device: "<<szText.c_str());

            bool bret;
            bret = report_center.sendToServer(SERVICE_CODE_CENTER, MINCODE_CENTER, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to center service error, data:"<<szText.c_str());
                printf("send to data service fail\n");
            }

            bret = report_block.sendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
            if(!bret)
            {
                LOG_ERROR("DetectClose: send data to block service error, data:"<<szText.c_str());
                printf("send to block service fail\n");

            }

            mapDev::iterator itFind = g_mapUnRegist.find(iter->first);
            if(itFind != g_mapUnRegist.end())
            {
                g_mapUnRegist.erase(itFind);
            }
            g_mapUnRegKeep.erase(iter++);
            continue;			
        }

        iter ++;
    }

    g_mapUnRegKeep.swap(g_mapUnRegist);
    g_mapUnRegist.clear();

    //report_center.close();
    //report_block.close();	

    printf("keep count = %lu\n", g_mapUnRegKeep.size());

    return 0;
}

