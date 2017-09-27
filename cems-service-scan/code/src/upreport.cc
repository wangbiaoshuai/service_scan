#include "upreport.h"

#include "ConfigService.h"
#include "common_function.h"
#include "parse_configure.h"
#include "log.h"
#include "gen_algorithm.h"

#include <string>

namespace cems{ namespace service{ namespace scan{

CUpReport::CUpReport() :
m_szIp(""),
m_port(0),
m_is_open(false),
m_pclient(NULL), 
m_zipMode(0), 
m_encryptMode(0)
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

    m_zipMode 	= atoi(szCompress.c_str());
    m_encryptMode 	= atoi(szEncrypt.c_str());
}

CUpReport::~CUpReport()
{
}

bool CUpReport::init(std::string szIp, unsigned int port)
{
    m_szIp = szIp;
    m_port = port;

    return 1;
}

bool CUpReport::open()
{
    if(m_is_open)
    {
        LOG_WARN("report has opened.");
        return true;
    }
    if(m_szIp.empty() || m_port == 0)
    {
        LOG_ERROR("open: ip/port is error.");
        return false;
    }
    m_socket = boost::shared_ptr<TSocket>(new TSocket(m_szIp.c_str(), m_port));
    m_socket->setConnTimeout(1000 * 5); // set connection timeout 5S
    m_transport = boost::shared_ptr<TTransport>(new TFramedTransport(m_socket));
    m_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));
    m_pclient = new CommonServiceClient(m_protocol);

    try
    {
        m_transport->open();
    }
    catch(TException & tx)
    {
        LOG_ERROR("open: connect ip=" << m_szIp << ", port=" << m_port << "open-ERROR:" << tx.what());

        if(m_pclient != NULL)
        {
            delete m_pclient;
            m_pclient = NULL;
        }

        return false;
    }

    LOG_INFO("open: connect ip="<<m_szIp<<", port:"<<m_port<<" success.");
    return true;
}

bool CUpReport::close()
{
    bool bret = true;
    m_is_open = false;
    try
    {
        m_transport->close();
    }
    catch(TException & tx)
    {
        LOG_ERROR("CUpReport::close error("<<tx.what()<<")");
        bret = false;
    }

    if(m_pclient != NULL)
    {
        delete m_pclient;
        m_pclient = NULL;
    }

    LOG_INFO("close "<<m_szIp.c_str()<<":"<<m_port<<" report success.");
    return bret;
}

bool  CUpReport::sendToServer(std::string maxCode, std::string minCode, std::string checkCode, bool bzip, std::string szJdata )
{
    CGenAlgori genrial;
    std::string szRet;
    bool bret = false;

    //CUpReport::open();
    try
    {
        if(m_pclient)
        {
            bzip = 1;
            bool bEncrypt = 1;

            std::string  szSend;
            std::string  szKey = GenerteKey();
            unsigned int flag = GetRandomInteger(1, 30);

            szSend = szJdata;

            if(m_zipMode != 0)
            {
                szSend = genrial.Compress(szJdata, m_zipMode);
            }
            else
            {
                bzip = 0;
            }

            ST_ENCRYPT encrypt;
            EncryptMode(flag, szKey, encrypt);

            if(m_encryptMode != 0)
            {
                szSend = genrial.EnCrypt(szSend, encrypt.mode, encrypt.szKey);
            }
            else
            {
                flag = 5;  bEncrypt = 0;
            }

            checkCode = calCRC(szSend);
            m_pclient->getDataTS(szRet, maxCode, minCode, checkCode, bzip, szSend, bEncrypt, szKey, flag);

            if(m_encryptMode != 0)
            {
                szRet = genrial.DeCrypt(szRet, encrypt.mode, encrypt.szKey);
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
        }
    }
    catch(TException & tx)
    {
        LOG_ERROR("sendToServer:Exception:"<<tx.what()<<", maxCode="<<maxCode<<", serverip="<<m_szIp<<", port="<<m_port);

        bret =  false;
    }
    //CUpReport::close();

    return bret;
}

bool  CUpReport::sendToServerOnce(std::string maxCode, std::string minCode, std::string checkCode, bool bzip, std::string szJdata )
{
    boost::shared_ptr<TSocket> socket(new TSocket(m_szIp.c_str(), m_port));
    socket->setConnTimeout(1000 * 5); // set connection timeout 5S
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    /*char buffer[1024] = {0};
    sprintf(buffer, "send: ip = %s, port = %d, maxCode = %s , minCode = %s, jdata = %s\n", m_szIp.c_str(), m_port, maxCode.c_str(), minCode.c_str(), szJdata.c_str());
    printf("%s\n", buffer);
    log.WriteLog(buffer, LOG_INFO);*/
    LOG_INFO("sendToServerOnce: send: ip="<<m_szIp<<", port"<<m_port<<", maxCode="<<maxCode<<", minCode="<<minCode<<", jdata="<<szJdata);

    CommonServiceClient client(protocol);
    try
    {
        transport->open();

        std::string szRet;
        client.getDataTS(szRet, maxCode, minCode, checkCode, bzip, szJdata, false, "", 1);

        /*std::string szRecv;
        szRecv += "receive:";
        szRecv += szRet;
        log.WriteLog(szRecv.c_str(), LOG_INFO);

        printf("%s\n", szRet.c_str());*/
        LOG_INFO("sendToServerOnce:receive="<<szRet);

        transport->close();
    }
    catch(TException & tx)
    {
        /*sprintf(buffer, "%s send-ERROR: %s serverip = %s, port = %d", maxCode.c_str(), tx.what(), m_szIp.c_str(), m_port);
        log.WriteLog(buffer, LOG_ERROR);

        printf("ERROR: %s\n", tx.what());*/
        LOG_ERROR("sendToServerOnce:"<<maxCode<<" send-ERROR: "<<tx.what()<<" serverip="<<m_szIp<<", port="<<m_port);
        return false;
    }

    return true;
}
}}}
