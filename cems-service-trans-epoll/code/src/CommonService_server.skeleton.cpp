// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "CommonService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/server/TThreadedServer.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "log.h"
#include "service_reg.h"
#include "defines.h"
#include "common_function.h"
#include "gen_algorithm.h"
#include "parse_configure.h"
#include "json/json.h"
#include "CRC32.h"
#include "parse_policy.h"
#include "transfer_server.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::com::vrv::cems::common::thrift::service;
using namespace cems::service::scan;
using namespace std;

#define MAXCODE_TRANS_SERVICE "00FF1400"
#define MINCODE_TRANS_POLICY "1000"
#define SCREEN_PORT 9011

const char* log_file = "/tmp/cemstrans.stdout";

class CommonServiceHandler : virtual public CommonServiceIf 
{
public:
    CommonServiceHandler() 
    {
        // Your initialization goes here
    }

    void getDataTS(std::string& _return, const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& data, const bool isEncrypt, const std::string& key, const int32_t flag);

    void getDataTC(std::string& _return, const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& data, const std::string& sessionId, const int32_t msgCode) 
    {
        // Your implementation goes here
        printf("getDataTC\n");
    }

    int GetCompressMode();
    std::string WriteResult(std::string maxCode, std::string minCode, std::string ret, std::string desc, std::string jdata);
    std::string EncryptResult(std::string szRet, bool isEncrypt, bool isZip, ST_ENCRYPT & encrypt, int mode);
    bool IsValidCrc(const std::string& szCrc, const std::string& szjdata);
    bool IsPolicy(const std::string& maxCode, const std::string& minCode);
};

void CommonServiceHandler::getDataTS(std::string& _return, const std::string& maxCode, const std::string& minCode, const std::string& checkCode, const bool isZip, const std::string& data, const bool isEncrypt, const std::string& key, const int32_t flag)
{
    LOG_INFO("getDataTS: start.");
    ST_ENCRYPT encrypt;
    EncryptMode(flag, key, encrypt);
    int mode = GetCompressMode();

    //功能号校验
    if(!IsPolicy(maxCode, minCode))
    {
        _return = WriteResult(maxCode, minCode,  "1",  "功能号调用错误。", "");
        _return = EncryptResult(_return, isEncrypt, isZip, encrypt, mode);
        LOG_ERROR("getDataTS: maxCode or minCode is error: maxCode="<<maxCode<<", minCode="<<minCode);
        return;
    }

    //CRC校验
    if(!IsValidCrc(checkCode, data))
    {
        _return = WriteResult(maxCode, minCode,  "1",  "CRC 校验失败。",
                "");
        _return = EncryptResult(_return, isEncrypt, isZip, encrypt, mode);
        LOG_ERROR("getDataTS: check CRC failed.");
        return;
    }

    CGenAlgori genrial;
    string recv_data = data;
    if(isEncrypt)
    {
        recv_data = genrial.DeCrypt(data, encrypt.mode, encrypt.szKey);
    }

    if(isZip)
    {
        recv_data = genrial.UnCompress(recv_data, mode);
    }

    if(IsPolicy(maxCode, minCode))
    {
        if(ParsePolicy::GetInstance().WritePolicy(recv_data) != -1)
        {
            LOG_INFO("getDataTS: update policy file success.");
            _return = WriteResult(maxCode, minCode,  "0",  "功能调用成功。", "");
            ParsePolicy::GetInstance().SetLogLevel();
        }
        else
        {
            LOG_ERROR("getDataTS: update policy file error.");
            _return = WriteResult(maxCode, minCode,  "1",  "解析xml失败。", "");
        }
    }
    
    _return = EncryptResult(_return, isEncrypt, isZip, encrypt, mode);
    LOG_INFO("getDataTS: end.");
    return;
}

bool CommonServiceHandler::IsPolicy(const std::string& maxCode, const std::string& minCode)
{
    if(maxCode != MAXCODE_TRANS_SERVICE || minCode != MINCODE_TRANS_POLICY)
    {
        return false;
    }
    return true;
}

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

#if 0
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
 
    //8)处理SIGCHLD信号
    signal(SIGCHLD, SIG_IGN);

    //2)后台运行
    pid = fork();
    if(pid != 0)
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
    pid = fork();
    if(pid != 0)
    {
        exit(0);
    }
    else if(pid < 0)
    {
        perror("fork2");
        exit(EXIT_FAILURE);
    }

    //5)关闭打开的文件描述符
    for(int i = 3; i < NOFILE; i++)
    {
        close(i);
    }

    //6)改变当前工作目录
    chdir(cur_path.c_str());

    //7)重新设置文件创建掩码
    umask(0);

    return 0;
}
#endif
#include <signal.h>
#include <sys/stat.h>
int init_daemon(void)  //创建守护进程
{
    setvbuf(stdout, NULL, _IOLBF, 0); //设置stdout的缓冲类型为行缓冲
    int pid;
    string cur_path = GetCurrentPath();

    //1)屏蔽一些阻断信号
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
 
    //8)处理SIGCHLD信号
    //signal(SIGCHLD, SIG_IGN);

    //2)后台运行
    pid = fork();
    if(pid > 0)
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
    /*pid = fork();
    if(pid > 0)
    {
        exit(0);
    }
    else if(pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }*/

    //5)关闭打开的文件描述符
    for(int i = 0; i < NOFILE; i++)
    {
        close(i);
    }

    //6)改变当前工作目录
    chdir(cur_path.c_str());

    //7)重新设置文件创建掩码
    umask(0);

    //9)重定向标准输出
    int fd = creat(log_file, 0644);
    if(fd < 0)
    {
        perror("creat");
    }
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);

    return 0;
}

//extern int StartTrans();
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

    TransferServer transfer_server(SCREEN_PORT);
    if(transfer_server.Start() < 0)
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
    try
    {
        server.serve();
    }
    catch(TTransportException te)
    {
        string exception(te.what());
        LOG_ERROR("main: TTransportException("<<exception.c_str()<<".)");
        return -1;
    }
    catch(...)
    {
        LOG_ERROR("main: catch an Exception.");
        return -1;
    }

    service_reg.Stop();
    LOG_INFO("service end");
    return 0;
}

