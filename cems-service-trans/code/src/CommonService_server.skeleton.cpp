// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "CommonService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/server/TThreadedServer.h>

#include "log.h"
#include "service_reg.h"
#include "defines.h"
#include "common_function.h"
#include "gen_algorithm.h"
#include "parse_configure.h"
#include "json/json.h"
#include "CRC32.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::com::vrv::cems::common::thrift::service;
using namespace cems::service::scan;

#define COMPRESS_MODE 1

class CommonServiceHandler : virtual public CommonServiceIf 
{
public:
    CommonServiceHandler() 
    {
        // Your initialization goes here
    }

    void getDataTS(std::string& _return, const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& data, const bool isEncrypt, const std::string& key, const int32_t flag)
    {
    }

    void getDataTC(std::string& _return, const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& data, const std::string& sessionId, const int32_t msgCode) 
    {
        // Your implementation goes here
        printf("getDataTC\n");
    }

    int GetCompressMode();
    std::string WriteResult(std::string maxCode, std::string minCode, std::string ret, std::string desc, std::string jdata);
    std::string EncryptResult(std::string szRet, bool isEncrypt, bool isZip, ST_ENCRYPT & encrypt, int mode);
    bool IsValidCrc(const std::string& szCrc, const std::string& szjdata);
};

int CommonServiceHandler::GetCompressMode()
{
    int mode = 0;
    string key = "service.compressmode";
    string value;
    if(ParseConfigure::GetInstance().GetProperty(key, value) && !value.empty())
    {
        mode = atoi(value.c_str());
    }
    return mode;
}

std::string CommonServiceHandler::WriteResult(std::string maxCode, std::string minCode, std::string ret, std::string desc, std::string jdata)
{
    Json::Value root;
    root["maxCode"] = Json::Value(maxCode);
    root["minCode"] = Json::Value(minCode);
    root["result"] = Json::Value(ret);
    root["desc"] = Json::Value(desc);

    Json::Value jarray;
    Json::Value temp;
    temp[""] = Json::Value(jdata);
    jarray.append(jdata);
    root["jdata"] = jarray;
    Json::FastWriter writer;
    std::string szRet = writer.write(root);
    return szRet;
}

std::string CommonServiceHandler::EncryptResult(std::string szRet, bool isEncrypt, bool isZip, ST_ENCRYPT & encrypt, int mode)
{
    std::string szReturn = szRet;
    CGenAlgori genrial;
    if(isZip)
    {
        if(mode == 1 || mode == 2) //deflate gzip
        {
            szReturn = genrial.Compress(szReturn, mode);
        }
    }
    if(isEncrypt)
    {
        szReturn = genrial.EnCrypt(szReturn, encrypt.mode, encrypt.szKey);
    }
    return szReturn;
}

bool CommonServiceHandler::IsValidCrc(const std::string& szCrc, const std::string& szjdata)
{
    unsigned int uCrc;
    std::string szDataCRC;
    StringCrc32((const unsigned char*)szjdata.c_str(), &uCrc, (int)szjdata.length());
    
    char buf[50] = {0};
    sprintf(buf, "%08X", uCrc);
    szDataCRC = buf;

    if(szDataCRC.compare(szCrc) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#include <signal.h>
#include <sys/stat.h>
int init_daemon(void)  //创建守护进程
{
    int pid;
    string cur_path = GetCurrentPath();

    //1)屏蔽一些阻断信号
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    //2)后台运行
    if(pid = fork())
    {
        exit(0);
    }
    else if(pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    //3)脱离控制终端、登录会话和进程组
    setsid();

    //4)禁止进程重新打开控制终端
    if(pid = fork())
    {
        exit(0);
    }
    else if(pid < 0)
    {
        perror("fork2");
        exit(EXIT_FAILURE);
    }

    //5)关闭打开的文件描述符
    for(int i = 0; i < NOFILE; i++)
    {
        close(i);
    }

    //6)改变当前工作目录
    chdir(cur_path.c_str());

    //7)重新设置文件创建掩码
    umask(0);

    //8)处理SIGCHLD信号
    signal(SIGCHLD, SIG_IGN);
    return 0;
}

extern int StartTrans();
int main(int argc, char **argv) 
{
    init_daemon();
    INIT_LOG(LOG_CONFIG_PATH);
    LOG_INFO("begin main");

    ServiceReg service_reg;
    if(service_reg.Start() == false)
    {
        LOG_ERROR("main: register serivce start failed");
        return -1;
    }
    if(StartTrans() != 0)
    {
        LOG_ERROR("main: StartTrans failed.");
        return -1;
    }

    int port = service_reg.GetServicePort();
    shared_ptr<CommonServiceHandler> handler(new CommonServiceHandler());
    shared_ptr<TProcessor> processor(new CommonServiceProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TFramedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
    server.serve();

    service_reg.Stop();
    LOG_INFO("service end");
    return 0;
}
