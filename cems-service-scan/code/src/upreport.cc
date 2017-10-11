#include "upreport.h"
#include "parse_configure.h"
#include "common_function.h"
#include "log.h"

namespace cems{ namespace service{ namespace scan{

UpReport::UpReport():
trans_pool_(),
server_ip_(""),
server_port_(0),
trans_num_(0),
is_open_(false),
zip_mode_(0),
encrypt_mode_(1),
genrial_()
{
    std::string szPath = GetCurrentPath();

    size_t index = szPath.rfind("/");
    if(index != std::string::npos)
    {
        szPath.erase(index + 1);
    }
    szPath += "config/";
    szPath += CONFIG_CONFIG_PROPERTIES;	

    std::string szCompress;
    std::string szEncrypt;

    ParseConfigure::GetInstance().GetProperty("compress", szCompress);
    ParseConfigure::GetInstance().GetProperty("encrypt", szEncrypt);

    if(szEncrypt.empty())
    {
        szEncrypt = "1";
    }

    if(szCompress.empty())
    {
        szCompress = "0";
    }

    zip_mode_ = atoi(szCompress.c_str());
    encrypt_mode_ = atoi(szEncrypt.c_str());
}

UpReport::~UpReport()
{
}

bool UpReport::Init(const std::string& ip, unsigned int port, int trans_num)
{
    if(ip.empty() || port <= 0)
    {
        return false;
    }
    server_ip_ = ip;
    server_port_ = port;
    trans_num_ = trans_num;
    return true;
}

bool UpReport::Open()
{
    int res = trans_pool_.Init(server_ip_, server_port_, trans_num_);
    if(res < 0)
    {
        is_open_ = false;
        return false;
    }
    is_open_ = true;
    return true;
}

void UpReport::Close()
{
    trans_pool_.Destroy();
}

bool UpReport::SendToServer(std::string maxCode, std::string minCode, std::string checkCode, bool bzip, std::string szJdata )
{
    if(!is_open_)
    {
        LOG_ERROR("SendToServer: open error.");
        return false;
    }

    std::string szRet;
    bool bret = false;
    Transport* trans = NULL;

    try
    {
        trans = trans_pool_.GetTransport();
        if(trans == NULL)
        {
            LOG_ERROR("SendToServer: get transport failed.");
            return false;
        }
        bzip = 1;
        bool bEncrypt = 1;

        std::string  szSend;
        std::string  szKey = GenerteKey();
        unsigned int flag = GetRandomInteger(1, 30);

        szSend = szJdata;

        if(zip_mode_ != 0)
        {
            szSend = genrial_.Compress(szJdata, zip_mode_);
        }
        else
        {
            bzip = 0;
        }

        ST_ENCRYPT encrypt;
        EncryptMode(flag, szKey, encrypt);

        if(encrypt_mode_ != 0)
        {
            szSend = genrial_.EnCrypt(szSend, encrypt.mode, encrypt.szKey);
        }
        else
        {
            flag = 5;  bEncrypt = 0;
        }

        checkCode = calCRC(szSend);
        trans->m_pclient->getDataTS(szRet, maxCode, minCode, checkCode, bzip, szSend, bEncrypt, szKey, flag);

        if(encrypt_mode_ != 0)
        {
            szRet = genrial_.DeCrypt(szRet, encrypt.mode, encrypt.szKey);
        }	

        /*sprintf(buffer, "发送数据 checkCode = %s,  szKey = %s, flag = %d", checkCode.c_str(), szKey.c_str(), flag);
          log.WriteLog(buffer, LOG_INFO);

          printf("serverip = %s, port = %d, maxCode = %s , minCode = %s\n", m_szIp.c_str(), m_port, maxCode.c_str(), minCode.c_str());
          printf("receive = %s, send = %s\n", szRet.c_str(), szJdata.c_str());

          sprintf(buffer, "serverip = %s, port = %d, maxCode = %s , minCode = %s", m_szIp.c_str(), m_port, maxCode.c_str(), minCode.c_str());
          log.WriteLog(buffer, LOG_INFO);	*/
        //LOG_INFO("sendToServer: send data checkCode="<<checkCode<<", szKey="<<szKey<<", flag="<<flag);
        //LOG_INFO("sendToServer:serverip="<<m_szIp<<", port="<<m_port<<", maxCode="<<maxCode<<", minCode="<<minCode);

        /*std::string szOut;
          szOut = "send = ";
          szOut += szJdata;
          log.WriteLog(szOut, LOG_INFO);


          std::string szRecv;
          szRecv += "receive:";
          szRecv += szRet;
          log.WriteLog(szRecv, LOG_INFO);*/
        //LOG_INFO("sendToServer:send="<<szJdata);
        //LOG_INFO("sendToServer:receive="<<szRet);
        //LOG_DEBUG("sendToServer: serverip("<<m_szIp<<":"<<m_port<<") send data("<<szJdata<<") success.");

        bret = true;
        trans_pool_.FreeTransport(trans);
    }
    catch(TTransportException te)
    {
        string exception(te.what());
        LOG_ERROR("sendToServer: TTransportException("<<exception.c_str()<<").");
        trans_pool_.DeleteTransport(trans);
        bret = false;
    }
    catch(TException tx)
    {
        string exception(tx.what());
        LOG_ERROR("sendToServer:Exception:"<<exception.c_str()<<", maxCode="<<maxCode<<", serverip="<<server_ip_<<", port="<<server_port_);
        trans_pool_.DeleteTransport(trans);

        bret =  false;
    }

    return bret;
}
}}}
