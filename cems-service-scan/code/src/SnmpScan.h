#ifndef _M_SNMP_SCAN
#define _M_SNMP_SCAN

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <string>
#include "SnmpReport.h"

namespace cems{ namespace service{ namespace scan{
struct  snmpHeader
{
	unsigned int flag; 	  //vrv标记
	unsigned int contentSize; //数据大小（json）
};

struct threadParam
{
	void* param1;
	void* param2;
	int param3;
};

class SnmpScan
{
public:
	SnmpScan();
	~SnmpScan();

public:
	int  run();
	void work(int new_fd);
	void init(char* ip, int port, char* szCert, char* szPrivKey);
	void GetServiceInfo(std::string ServiceCode, std::string& szIp, std::string& szPort);

private: 
	std::string 	m_szIp;
	int 		m_port;

	std::string 	m_szCert;
	std::string	m_szPrivKey;

	int		m_listenNum;

private:
	SnmpReport	m_Report;
	
};
}}}
#endif
