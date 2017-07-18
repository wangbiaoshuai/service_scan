#ifndef		DEFINES_H_
#define		DEFINES_H_

#include <map>
#include <string>
#include <list>

#define USHORT   unsigned short
#define ULONG    unsigned long
#define UCHAR    unsigned char
#define BYTE 	 UCHAR

#define MAXINTERFACES    16
#define NBNS_PORT        137
#define CLIENT_PORT      22116

#define CACL_SIZE(T)    (sizeof(T)/sizeof(T[0]))

using namespace std;

struct IP_RANGE
{
    ULONG first;
    ULONG second;
};

struct  DEV_INFO
{
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
    int    count;
};

struct THD_PARAM
{
   void* param1;
   void* param2;
};

struct Q_NETBIOSNS
{
    USHORT  tid;
    USHORT  flags;
    USHORT  questions;
    USHORT  answerRRS;
    USHORT  authorityRRS;
    USHORT  additionalRRS;
    UCHAR   name[34];
    USHORT  type;
    USHORT  classe;
};

struct  ST_PARAMS
{
    std::string szIpRange;
    std::string szOrgid;
    unsigned int  uScanType;
    std::string szIntervalTime;
    std::string szUdpTime;
    std::string szClientPort;
    std::string szAreaId;
};

struct THRD_PARAM
{
        std::string start;
        std::string end;
        void* param;
};

struct ST_COMMON
{
        void* p1;
        void* p2;

};

enum  SRV_TYPE
{
        SRV_TOPO = 1, //拓扑
        SRV_BLOCK,    //阻断
        SRV_CENTER    //数据中心
};


typedef struct _CEMS_NET_HEAD
{
	unsigned int dwFlag;   
        unsigned int dwVersion; 
	unsigned int dwDataSize; 
	unsigned int dwCrc;

	unsigned char szSessionId[16];
	unsigned int  dwMsgCode;

        unsigned int dwMaxCode;
        unsigned int dwMinCode;

	unsigned short wHeadSize;
	unsigned short wType;
	unsigned short wCount;
	unsigned short wIndex;
	unsigned char  szAreaId[32];

}CEMS_UDP_HEAD,*PCEMS_UDP_HEAD;

struct ST_ENCRYPT
{
        ST_ENCRYPT()
        {
                mode = 0;
                szKey = "";
        }
        int mode;
        std::string szKey;
};

#define UDP_MAXCODE    (0x00FF0200)
#define UDP_MINCODE    (0x00FF0002)

#define         SCAN_SERVER_FLAG        "ScanServerPacket"

typedef map<ULONG, DEV_INFO> mapDev;
typedef map<ULONG, ULONG> mapFLAG;
typedef list<mapDev> LIST_DEV;

typedef std::map<std::string, std::string> MAP_COMMON;
typedef std::list<MAP_COMMON> LIST_KEYS;

#define LOG4J_LOGPATH              "log4j.appender.FILE.File"

#define CONFIG_LOG4J_PROPERTIES    "log4j.properties"
#define CONFIG_CONFIG_PROPERTIES   "config.properties"
#define CONFIG_POLICY_XML	   "policy.xml"

#define SHELL_SERVICE_RESTART	   "service CEMS-SERVICE-SCAN restart" 

#define SERVICE_CODE_CONFIG     "00FF0100"          //配置服务
#define SERVICE_CODE_BLOCK      "00010400"          //阻断服务
#define SERVICE_CODE_CENTER     "00FF0A00"          //数据处理服务
#define SERVICE_CODE_TOPO       "00010200"          //拓扑服务
#define MINCODE_CENTER          "2100"                  
#define MINCODE_BLOCK           "2100"

#define SERVICE_CODE_ADDRESS	"00FF0600"	    //地址服务
#define MINCODE_ADDRESS_AREAID		"8"		    //获取AreaId
#define MINCODE_ADDRESS_UPDATEURL       "5"		    //获取策略更新地址

#endif
