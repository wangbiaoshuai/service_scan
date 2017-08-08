#ifndef _M_REPORT
#define _M_REPORT

#include "CommonService.h"
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TBinaryProtocol.h>

namespace cems{ namespace service{ namespace scan{

using namespace boost;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

//using namespace  ::com::vrv::im::service;
using namespace com::vrv::cems::common::thrift::service;
using boost::shared_ptr;

class CUpReport
{
public:
	CUpReport();
	~CUpReport();
public:
	bool  init(std::string szIp, unsigned int port);
	bool  open();
	bool  close();
	bool  sendToServer(std::string maxCode, std::string minCode, std::string checkCode, bool bzip, std::string szJdata);
	bool  sendToServerOnce(std::string maxCode, std::string minCode, std::string checkCode, bool bzip, std::string szJdata );

private:
	std::string m_szIp;
	unsigned int m_port;

private:
	boost::shared_ptr<TSocket>     m_socket; 
	boost::shared_ptr<TTransport>  m_transport;
	boost::shared_ptr<TProtocol>   m_protocol;	

	CommonServiceClient*  m_pclient;

public:
	int	m_zipMode;
	int 	m_encryptMode;

};
}}}
#endif
