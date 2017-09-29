#ifndef _M_SNMP_REPORT
#define _M_SNMP_REPORT

#include <string>
#include <map>

#include "ConfigService.h"
#include "upreport.h"

namespace cems{ namespace service{ namespace scan{
typedef std::map<std::string, int> MAP_IP_STATE;

class SnmpReport
{
public:
	SnmpReport();
	~SnmpReport();

public:
	bool send(std::string szJson);

public:
	bool init(std::string szCenterIp, std::string szCenterPort, std::string szBlockIp, std::string szBlockPort);

private:
	std::string m_szCenterIp;
	int  m_centerPort;
	std::string m_szBlockIp;
	int  m_blockPort;

private:
	std::string 	m_szAreaId;

private:
	UpReport	m_RptCenter;
	UpReport	m_RptBlock;

public:
	bool sendpack(int fd, char* ip);
	void recvClientPack(int sock, MAP_IP_STATE& RegDev);
public:
	void GetIpState(std::string szJson);

public:	
	int fd_reg;
	volatile int 	m_bStopRegScan;

	MAP_IP_STATE	m_ipState;
	MAP_IP_STATE	m_ipReg;
};
}}}
#endif
