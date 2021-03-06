#include "detect_host.h"

#include <sys/wait.h>

#include "json/json.h"
#include "conv.h"
#include "common_function.h"
#include "log.h"
#include "service_reg.h"
#include "arp.h"
#include "dev_identify.h"

using namespace cems::service::scan;
using namespace std;

#define THREAD_SLEEP_TIME	5*1000*1000
#define SOCKET_SELECT_TIMEOUT	3*1000*1000
#define PACKET_SIZE     4096
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
dev_manager_(),
area_id_(""),
org_id_(""),
register_scan_stop_(0),
icmp_scan_stop_(0),
nbt_scan_stop_(0),
pid_(0),
sockfd_(0),
sockud_(0),
sockcd_(0),
dep_scan_thread_(0),
dep_scan_stop_(0),
dep_scan_pause_(1)
{
}

DetectHost::~DetectHost()
{
    dep_scan_pause_ = 0;
    dep_scan_stop_ = 1;
}

void* dep_scan_function(void* context)
{
    DetectHost* ctx = static_cast<DetectHost*>(context);
    ctx->DepthScan();
    pthread_exit(NULL);
}

int DetectHost::Init()
{
    LOG_INFO("DetectInit: begin.");
    if(dev_manager_.InitReportObj() < 0)
    {
        LOG_ERROR("Init: init report object failed.");
    }

    if(dep_scan_thread_ == 0)
    {
        dep_scan_stop_ = 0;
        dep_scan_pause_ = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        int res = pthread_create(&dep_scan_thread_, &attr, dep_scan_function, this);
        if(res < 0)
        {
            LOG_ERROR("Start: create depth scan thread error("<<strerror(errno)<<").");
            pthread_attr_destroy(&attr);
            return -1;
        }
        pthread_attr_destroy(&attr);
    }

    LOG_INFO("Init: end.");
    return 	1;
}

void DetectHost::DetectChange()
{
    dev_manager_.ClearDevMap();
}

int DetectHost::Start(MAP_COMMON * ipRange, std::string szAreaId, std::string szOrgId, int mode)
{
    area_id_ = szAreaId;
    org_id_ = szOrgId;

    dev_manager_.SetDetectMode(mode);
    if(mode == 1)
    {    
        dep_scan_pause_ = 0;
        LOG_INFO("Start: depth scan continue...");
    }
    else
    {
        dep_scan_pause_ = 1;
        LOG_INFO("Start: depth scan pause...");
    }
    StandardScan(ipRange);

    return 0;
}

int DetectHost::DepthScan()
{
    LOG_INFO("DepthScan: begin.");
    while(!dep_scan_stop_)
    {
        while(dep_scan_pause_)
        {
            sleep(30);
        }

        DEV_INFO device;
        while(false == dev_manager_.GetDevfromNmap(device))
        {
            sleep(30);
        }

        if(!device.szDevType.empty() && !device.szOsType.empty())
        {
            LOG_DEBUG("DepthScan: device("<<device.szIP.c_str()<<")---->"<<device.szDevId.c_str());
            continue;
        }

        int pid = fork();
        if(pid == 0)
        {
            devscan_result_s dev_info = {0};
            int res = devscan_identify(inet_addr(device.szIP.c_str()), &dev_info);
            if(res < 0)
            {
                LOG_WARN("DepthScan: nmap scan deivce("<<device.szIP.c_str()<<") failed.");
                exit(-1);
            }
            if(dev_info.status == 0)
            {
                LOG_WARN("DepthScan: device("<<device.szIP.c_str()<<") is unreachable.");
                exit(-1);
            }
            device.szDevType = dev_info.devtype_cn_name;
            device.szOsType = dev_info.systype_name;
            //进行上报
            LOG_DEBUG("DepthScan: nmap scan ip("<<device.szIP.c_str()<<")---->"<<device.szDevType.c_str());
            dev_manager_.ReportNmapDev(device);
            exit(0);
        }
        else if(pid < 0)
        {
            LOG_ERROR("DepthScan: fork error("<<strerror(errno)<<").");
            continue;
        }
        else
        {
            wait(NULL); //防止信号中断该函数，导致子进程回收资源失败。
        }
    }
    dev_manager_.ClearNmap();
    LOG_INFO("DepthScan: end.");
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
    dev_manager_.UpdateRegKeepDev();
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

    std::string szText = CreateSendText(devinfo);
    LOG_DEBUG("parseClientPack: discover register device: "<<szText.c_str());
    dev_manager_.PushRegDev(uip, devinfo);

    if(devinfo.szAreaId.compare(devinfo.szRegAreaId) != 0) //漫游主机(区域id不同)
    {
        dev_manager_.PushRoamingDev(uip, devinfo);
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
                DEV_INFO device;
                device.szIP = ip;
                device.szBoot = "1";
                device.szAreaId = area_id_;
                device.szOrgId  = org_id_;
                device.szRegAreaId = area_id_;
                device.szRegOrgId = org_id_;
                device.szFireWall = "2";
                device.count = 0;

                dev_manager_.PushUnregDev(uip, device);
            }
        }
    }
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

            dev_manager_.PushUnregDev(ipRev, device);
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
            return false;
        }
    }

    if(type == s_ping)
    {
        int packetsize = MakeIcmpPack(i++, (struct icmp*)sendpacket); //设置ICMP报头

        if((len = sendto(sockfd_, sendpacket, packetsize, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr))) < 0  )
        {
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
        LOG_ERROR("ClientProbe: create thread failed.("<<strerror(rc)<<")");
    }

    rc = pthread_create(&t_recv, NULL, RecvRegProc, (void*)this);
    if(rc != 0)
    {
        LOG_ERROR("ClientProbe: create thread failed.("<<strerror(rc)<<")");
    }

    rc = pthread_join(t_send, NULL);
    if(rc != 0)
    {
        LOG_ERROR("ClientProbe: pthread_join error.("<<strerror(rc)<<")");
    }

    rc = pthread_join(t_recv, NULL);
    if(rc != 0)
    {
        LOG_ERROR("ClientProbe: pthread_join error.("<<strerror(rc)<<")");
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
        LOG_ERROR("IcmpProbe: socket raw create failed.");
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
        LOG_ERROR("IcmpProbe: create thread failed.("<<strerror(rc)<<")");
    }
    rc = pthread_create(&t_recvPing, NULL, RecvPingProc, (void*)this);
    if(rc != 0)
    {
        LOG_ERROR("IcmpProbe: create thread failed.("<<strerror(rc)<<")");
    }
    rc = pthread_join(t_sendDetec, NULL);
    if(rc != 0)
    {
        LOG_ERROR("IcmpProbe: pthread_join error.("<<strerror(rc)<<")");
    }
    rc = pthread_join(t_recvPing, NULL);
    if(rc != 0)
    {
        LOG_ERROR("IcmpProbe: pthread_join error.("<<strerror(rc)<<")");
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
        LOG_ERROR("NbtProbe: socket udp create failed.");
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
        LOG_ERROR("NbtProbe: create thread failed.("<<strerror(rc)<<")");
    }
    rc = pthread_create(&t_recvNbt, NULL, RecvNbtProc, (void*)this);
    if(rc != 0)
    {
        LOG_ERROR("NbtProbe: create thread failed.("<<strerror(rc)<<")");
    }
    rc = pthread_join(t_sendDetec, NULL);
    if(rc != 0)
    {
        LOG_ERROR("NbtProbe: pthread_join error.("<<strerror(rc)<<")");
    }

    rc = pthread_join(t_recvNbt, NULL);
    if(rc != 0)
    {
        LOG_ERROR("NbtProbe: pthread_join error.("<<strerror(rc)<<")");
    }
    close(sockud_);
    LOG_INFO("NbtProbe: end.");
    return 0;
}

int DetectHost::StandardScan(MAP_COMMON * ipRange)
{
    LOG_INFO("start standerd scan.");
    dev_manager_.GetRunawayDev();
    ClientProbe(ipRange);
    NbtProbe(ipRange);
    IcmpProbe(ipRange);
    LOG_INFO("standerd scan end.");
    return 0;
}

int DetectHost::CalculateCloseDev()
{
    return dev_manager_.CalculateCloseDev();
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
