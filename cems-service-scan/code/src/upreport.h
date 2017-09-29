#ifndef _UPREPORT_H
#define _UPREPORT_H

#include "transport_pool.h"

namespace cems{ namespace service{ namespace scan{ 
class UpReport
{
public:
    UpReport();
    ~UpReport();

    bool Init(const std::string& ip, unsigned int port, int trans_num = 0);
	bool Open();
	void Close();
	bool SendToServer(std::string maxCode, std::string minCode, std::string checkCode, bool bzip, std::string szJdata);

private:
    TransportPool trans_pool_;
    std::string server_ip_;
    unsigned int server_port_;
    int trans_num_;

private:
	int	zip_mode_;
	int encrypt_mode_;

};
}}}
#endif // _UPREPORT_H
