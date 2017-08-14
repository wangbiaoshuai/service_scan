#include "SnmpReport.h"

#include "service_reg.h"
#include "fastscan.h"
#include "defines.h"
#include "common_function.h"
#include "json/json.h"
#include "log.h"
#include "parse_configure.h"

namespace cems{ namespace service{ namespace scan{
#define THREAD_SLEEP_TIME	5*1000*1000
#define SOCKET_SELECT_TIMEOUT	3*1000*1000

SnmpReport::SnmpReport()
{
    m_bStopRegScan = 0;
}

SnmpReport::~SnmpReport()
{

}

std::string timeToString(int timeStamp)
{
    time_t time = timeStamp;
    struct tm *pTmp = localtime(&time); 
    if (pTmp == NULL) 
    { 
        return ""; 
    } 

    char buffer[100] = {0};
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", pTmp->tm_year + 1900, pTmp->tm_mon + 1, pTmp->tm_mday, pTmp->tm_hour, pTmp->tm_min, pTmp->tm_sec); 

    std::string szTime = buffer;
    return szTime; 
}

void MakeClientPackEx(char* sendBuffer, int & len)
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

bool  parseClientPackEx(ULONG uip,  char* data, int iLen)
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
        return false;
    }

    return true;
}

bool SnmpReport::sendpack(int fd, char* ip)
{
    char sendbuffer[1024] = {0};
    int  len = 1024;

    MakeClientPackEx(sendbuffer, len);

    struct sockaddr_in toAddr = {0};

    toAddr.sin_family = AF_INET;
    toAddr.sin_addr.s_addr = inet_addr(ip);
    toAddr.sin_port = htons(CLIENT_PORT);

    if((len = sendto(fd, sendbuffer, len, 0, (struct sockaddr *)&toAddr, sizeof(toAddr))) < 0)
    {
        printf("client udp sendto fail, size = %d\n", len);
        return false;		
    }

    return true;
}

//接受注册客户端udp回包
void SnmpReport::recvClientPack(int sock, MAP_IP_STATE& RegDev)
{
    int ret;
    fd_set fst;

    while(!m_bStopRegScan)
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
                bool bRet = parseClientPackEx(ntohl(addrfrom.sin_addr.s_addr), recvBuf, ret);
                if(bRet) //reg device
                {
                    std::string szip = inet_ntoa(addrfrom.sin_addr);
                    RegDev.insert(MAP_IP_STATE::value_type(szip, 0));
                }
            }
        }
    }
    printf("client exit\n");
}

void* SendRegProcEx(void* param)
{
    SnmpReport * pReport = static_cast<SnmpReport*>(param);

    pReport->m_bStopRegScan = 0;

    MAP_IP_STATE::iterator iter = pReport->m_ipState.begin();
    while(iter != pReport->m_ipState.end())
    {
        pReport->sendpack(pReport->fd_reg, (char*)iter->first.c_str());
        iter ++;
    }

    usleep(THREAD_SLEEP_TIME);
    pReport->m_bStopRegScan = 1;

    return (void*)NULL;
}

void*  RecvRegProcEx(void* param)
{
    SnmpReport * pReport = static_cast<SnmpReport*>(param);
    pReport->recvClientPack(pReport->fd_reg, pReport->m_ipReg);	
    return (void*)NULL;
}

void SnmpReport::GetIpState(std::string szJson)
{
    m_ipState.clear();
    m_ipReg.clear();

    Json::Value root;
    Json::Reader reader;
    if(!reader.parse(szJson, root))
    {
        return;
    }

    Json::Value jdata = root["data"];

    int size = jdata.size();
    for(int i = 0; i < size ; i++)
    {
        Json::Value info = jdata[i];

        std::string szType = info["devType"].asCString();
        std::string szIp = info["ip"].asCString(); 

        if(szType.compare("pc") == 0)
        {
            m_ipState.insert(MAP_IP_STATE::value_type(szIp, 0));
        }
    }

    if((fd_reg = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("socket udp create fail\n");
        return ;
    }

    pthread_t t_send;
    pthread_t t_recv;

    int rc;
    rc = pthread_create(&t_send, NULL, SendRegProcEx, (void*)this);
    if(rc != 0)
    {
        printf("create thread fail %s\n", strerror(rc));
    }

    rc = pthread_create(&t_recv, NULL, RecvRegProcEx, (void*)this);
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

    close(fd_reg);
}

bool SnmpReport::send(std::string szJson)
{
    GetIpState(szJson);

    Json::Value root;
    Json::Reader reader;
    if(!reader.parse(szJson, root))
    {
        return false;
    }

    Json::Value jdata = root["data"];
    int size = jdata.size();

    Json::Value jsend;
    Json::Value jdataSend;

    Json::FastWriter writer;
    std::string szText;

    for(int i = 0; i < size ; i++)
    {
        Json::Value info = jdata[i];
        Json::Value item;

        std::string szIp = info["ip"].asCString();
        std::string szType = info["devType"].asCString();

        if(m_ipReg.find(szIp) != m_ipReg.end())
        {
            continue; //ignore regdev	
        }		

        item["devType"]	= info["devType"];		
        item["osType"]	= info["osType"];				
        item["ip"] = info["ip"];				
        item["mac"] = info["mac"];			
        item["findTime"] = timeToString(atoi(info["timeStamp"].asCString()));	
        item["isOpened"] = strcmp(info["devState"].asCString(),"online") == 0 ? "1" : "0";
        item["orgId"] = "";		
        item["areaId"] = m_szAreaId;				
        item["isFireWall"] = "2";	

        jsend.append(item);
        jdataSend = jsend;

    }

    if(jdataSend.isNull())
    {
        return false;
    }

    szText = writer.write(jdataSend); 

    /*std::string szlog = "发现未注册主机 = ";
    szlog += szText;
    log.WriteLog(szlog, LOG_INFO);      	*/
    LOG_INFO("SnmpReport::send: find unregister device:"<<szText);

    //printf("%s\n", szlog.c_str());

    bool bret;
    bret = m_RptCenter.sendToServer(SERVICE_CODE_CENTER, MINCODE_SNMP, calCRC(szText), false, szText);
    if(!bret)
    {
        printf("send to data service fail\n");
        LOG_INFO("SnmpReport::send: send to center service failed. msg:"<<szText);
    }

    /*bret = m_RptBlock.sendToServer(SERVICE_CODE_BLOCK, MINCODE_BLOCK, calCRC(szText), false, szText);
      if(!bret)
      {
    //printf("send to block service fail\n");
    }*/

    return bret;
}

bool SnmpReport::init( std::string szCenterIp, std::string szCenterPort, std::string szBlockIp, std::string szBlockPort)
{
    LOG_INFO("SnmpReport::init begin.");
    string key = "service.serverAreaId";
    if(ParseConfigure::GetInstance().GetProperty(key, m_szAreaId) == false)
    {
        m_szAreaId = "serverAreaMain";
    }
    /*std::string szTemp;

    szTemp = "SnmpReport init config path ";
    szTemp += szConfigPath;
    log.WriteLog(szTemp, LOG_INFO);

    szTemp = "SnmpReport init  AreaId";
    szTemp += m_szAreaId;
    log.WriteLog(szTemp, LOG_INFO);*/

    ServiceReg cm;

    m_RptCenter.init(szCenterIp, atoi(szCenterPort.c_str()));
    m_RptBlock.init(szBlockIp, atoi(szBlockPort.c_str()));

    LOG_INFO("SnmpReport::init end.");
    return true;
}
}}}
