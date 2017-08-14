#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <pthread.h>

#include  "SnmpScan.h"
#include "service_reg.h"
#include "log.h"
#include "defines.h"

namespace cems{ namespace service{ namespace scan{
SSL_CTX *ctx;

SnmpScan::SnmpScan() : m_listenNum(1)
{

}

SnmpScan::~SnmpScan()
{

}

void SnmpScan::init(char* ip, int port, char* szCert, char* szPrivKey)
{
    m_port = port;

    if(ip != NULL)
    {
        m_szIp = ip;
    }

    if(szCert != NULL)
    {
        m_szCert = szCert;
    }

    if(szPrivKey != NULL)
    {
        m_szPrivKey = szPrivKey;
    }
}

void SnmpScan::GetServiceInfo(std::string ServiceCode, std::string& szIp, std::string& szPort)
{
    ServiceReg cm;

    //char buffer[1024] = {0};

    std::string szServiceIp;
    std::string szServicePort;
    std::string szServiceOrgId;

    if(cm.Fetch(ServiceCode, szServiceOrgId, szServiceIp, szServicePort))
    {
        /*sprintf(buffer, "fetch data service ip = %s, port = %s", szServiceIp.c_str(), szServicePort.c_str());
          log.WriteLog(buffer, LOG_INFO);

          printf("fetch data service ip = %s, port = %s\n", szServiceIp.c_str(), szServicePort.c_str());*/

        LOG_INFO("GetServiceInfo: fetch data service ip="<<szServiceIp.c_str()<<", port="<<szServicePort.c_str());
        szIp = szServiceIp;
        szPort = szServicePort;
    }
}

void* WorkProc(void* param)
{
    threadParam * tp = static_cast<threadParam*>(param);
    SnmpScan* pScan = (SnmpScan*)tp->param1;

    if(pScan)
    {
        pScan->work(tp->param3);
    }

    if(tp)
    {
        delete tp; tp = NULL;
    }

    return (void*)NULL;
}

void * SendProc(void* param)
{
    threadParam * tp = static_cast<threadParam*>(param);
    SnmpReport* pReport = (SnmpReport*)tp->param1;

    if(pReport)
    {
        std::string szJson;
        szJson.append((char*)tp->param2, tp->param3);

        pReport->send(szJson);

        delete [](char*)tp->param2;
        delete pReport; pReport = NULL;
    }

    if(tp)
    {
        delete tp; tp = NULL;
    }

    return (void*)NULL;
}

void SnmpScan::work(int new_fd)
{
    SSL *ssl;
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, new_fd);

    std::string szJson;
    snmpHeader header;

    std::string szCenterIp, szBlockIp;
    std::string szCenterPort, szBlockPort;

    if (SSL_accept(ssl) == -1) 
    {
        goto finish;
    }		

    GetServiceInfo(SERVICE_CODE_CENTER, szCenterIp, szCenterPort);
    GetServiceInfo(SERVICE_CODE_BLOCK, szBlockIp, szBlockPort);

    while(SSL_read(ssl, &header, sizeof(snmpHeader)) == sizeof(snmpHeader))
    {
        int flag = ntohl(header.flag);
        if(flag == 1) 	// 0 normal , 1  heart
        {
            /*char buffer[100] = {0};
            sprintf(buffer, "snmp heart , sock = %d", new_fd);
            log.WriteLog(buffer, LOG_INFO);*/
            LOG_INFO("snmp heart, sock="<<new_fd);

            continue;
        }

        int recvSize = ntohl(header.contentSize);
        char* pBuffer = new char[recvSize + 1];
        bzero(pBuffer, recvSize + 1);

        int len = SSL_read(ssl, pBuffer, recvSize);
        if(len == recvSize)
        {
            szJson = pBuffer;
            printf("recv = %s", pBuffer);

            SnmpReport* pReport = new SnmpReport;
            pReport->init(szCenterIp, szCenterPort, szBlockIp, szBlockPort);			

            threadParam* pparam = new threadParam;
            pparam->param1 = (void*)pReport;
            pparam->param2 = (void*)pBuffer;
            pparam->param3 = recvSize;			

            pthread_t tid;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            int rc = pthread_create(&tid, NULL, SendProc, (void*)pparam);
            if(rc != 0)
            {
                //printf("thread create failed");
                LOG_ERROR("create thread error.");
            }
            pthread_attr_destroy(&attr);
        }
    }

finish:		
    /* 关闭 SSL 连接 */
    SSL_shutdown(ssl);
    /* 释放 SSL */
    SSL_free(ssl);
    /* 关闭 socket */
    close(new_fd);
}

int SnmpScan::run()
{
    int sockfd;

    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(SSLv23_server_method());
    if (ctx == NULL) 
    {
        ERR_print_errors_fp(stdout);
        return -1;
    }

    /*SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);  

      if(!SSL_CTX_load_verify_locations(ctx, NULL, NULL))
      {
      printf("SSL_CTX_load_verify_locations error!\n");
      return NULL;
      } */

    if (SSL_CTX_use_certificate_file(ctx, m_szCert.c_str(), SSL_FILETYPE_PEM) <= 0) 
    {
        ERR_print_errors_fp(stdout);
        return -1;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, m_szPrivKey.c_str(), SSL_FILETYPE_PEM) <= 0) 
    {
        ERR_print_errors_fp(stdout);
        return -1;
    }

    if (!SSL_CTX_check_private_key(ctx)) 
    {
        ERR_print_errors_fp(stdout);
        return -1;
    }

    /* 开启一个 socket 监听 */
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
    {
        return -1;
    }

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(m_port);

    my_addr.sin_addr.s_addr = m_szIp.empty() ? INADDR_ANY : inet_addr(m_szIp.c_str());

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) 
    {
        close(sockfd);
        return -1;
    } 

    if (listen(sockfd, m_listenNum) == -1) 
    {
        close(sockfd);
        return -1;
    }

    while (1)
    {
        socklen_t len = sizeof(struct sockaddr);

        int new_fd;
        if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &len)) == -1) 
        {
            perror("accept");
            break;
        } 
        else
        {
            printf("server: got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port), new_fd);
            LOG_INFO("server: got connection from "<<inet_ntoa(their_addr.sin_addr)<<", port "<<ntohs(their_addr.sin_port)<<", socket "<<new_fd<<".");
        }

        //int flags = fcntl(fd_accept, F_GETFL, 0); 
        //int s_ret = fcntl(fd_accept, F_SETFL, flags | O_NONBLOCK);

        threadParam* ptparam = new threadParam;
        ptparam->param1 = (void*)this;
        ptparam->param2 = (void*)NULL;
        ptparam->param3 = new_fd;

        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
        int rc = pthread_create(&tid, &attr, WorkProc, (void*)ptparam);
        if(rc != 0)
        {
            printf("create thread failed");
        }
        pthread_attr_destroy(&attr);
    }

    close(sockfd);
    SSL_CTX_free(ctx);

    return 0;
}
}}}
/*int main()
  {
  SnmpScan scan;
  scan.init((char*)"192.168.88.77", 7838, (char*)"cacert.pem", (char*)"privkey.pem");

  scan.run();

  return 0;
  }*/
