#include "common_function.h"

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "CRC32.h"
#include "md5module.h"
#include "parse_configure.h"
#include "gen_algorithm.h"

#define MAXBUFSIZE 1024 
#define MAXINTERFACES 16

using namespace cems::service::scan;

void  itoa(unsigned long val,  char *buf, unsigned radix)
{
    char   *p;
    char   *firstdig;
    char   temp;
    unsigned   digval;

    p   =   buf;
    firstdig   =   p;

    do{
        digval =  (unsigned)(val % radix);
        val /=  radix;
        if(digval  >  9)
            *p++  = (char)(digval - 10  + 'a');
        else
            *p++ = (char)(digval + '0');
    } while (val > 0);

    *p-- = '\0';

    do{
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;
        --p;
        ++firstdig;
    }while(firstdig  < p);
}

void OutPutString(char* szContent)
{
    struct tm *ptm;
    long ts = time(NULL);

    ptm = localtime(&ts);

    char tmBuf[100] = {0};
    sprintf(tmBuf, "%04d-%02d-%02d %02d:%02d:%02d:\n", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    printf("%s", tmBuf);
    printf("%s\n", szContent);
}

bool  CreateDirectory(const char * sPathName)
{
    char   DirName[256];
    strcpy(DirName, sPathName);

    int   i;
    int  len;
    len = strlen(DirName);

    if(DirName[len-1] != '/')
        strcat(DirName, "/");

    len = strlen(DirName);

    for(i = 1; i < len; i ++)
    {
        if(DirName[i] == '/')
        {
            DirName[i] = 0;
            if(access(DirName, 0) != 0)
            {
                if(mkdir(DirName, 0755)==-1)
                {
                    perror("mkdir   error");
                    return   0;
                }
            }
            DirName[i] = '/';
        }
    }

    return   1;
}

std::string GetCurrentPath()
{
    std::string szPath;

    char buf[ MAXBUFSIZE ] = {0};
    int count;
    count = readlink( "/proc/self/exe", buf, MAXBUFSIZE );
    if ( count < 0 || count >= MAXBUFSIZE )
    {
        printf( "Failed\n" );
        return  szPath;
    }

    char* p = strrchr(buf, '/');
    if(p)
    {
        memset(p, 0, strlen(p));
    }

    szPath = buf;

    //printf("当前路径 : %s\n", szPath.c_str());
    return szPath;
}

std::string calCRC(std::string szText)
{
    unsigned int uCrc;
    std::string szDataCRC;

    StringCrc32((const unsigned char*)szText.data(), &uCrc, (int)szText.size());

    char buf[50] = {0};

    sprintf(buf, "%08X", uCrc);

    szDataCRC = buf;

    return szDataCRC;
}

std::string  GetConfigFilePath()
{
    std::string szPath = GetCurrentPath();

    size_t index = szPath.rfind("/");
    if(index != std::string::npos)
    {
        szPath.erase(index + 1);
    }

    szPath += "config/";
    szPath += CONFIG_CONFIG_PROPERTIES;

    return szPath;
}

std::string  GetStrMd5Ex(char* szSrc, int iLen)
{
    unsigned char szmd5sum[16];
    md5_context ctx;
    md5_starts(&ctx);
    md5_update(&ctx, (unsigned char *)szSrc, iLen);
    md5_finish(&ctx, szmd5sum);

    char szMd5[260] = {0};
    char szChar[3];
    for(int i = 0; i < 16; i ++)
    {
        if(i == 6)
        {
            szmd5sum[i] &= 0x0f;
            szmd5sum[i] |= 0x30;
        }
        if(i == 8)
        {
            szmd5sum[i] &= 0x3f;
            szmd5sum[i] |= 0x80;
        }

        itoa(szmd5sum[i], szChar, 16);
        if(strlen(szChar) == 1)
        {
            strcat((char*)szMd5,  "0");
        }
        strcat((char*)szMd5,  szChar);
    }

    std::string szResult = szMd5;

    return szResult;
}

std::string GenericUUID(std::string szKey)
{
    std::string sid = GetStrMd5Ex((char*)szKey.c_str(), szKey.length());
    return sid;
}

std::string GetCurrentIp()
{
    std::string szPath;
    szPath = GetConfigFilePath().c_str();

    std::string szKey;
    std::string szIp;

    szKey = "service.ip";
    if(ParseConfigure::GetInstance().GetProperty(szKey, szIp) && !szIp.empty())
    {
        return szIp;
    }

    int sock_fd;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    int interface_num;

    if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror("Create socket failed");

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_req = buf;
    if(ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc) < 0)
    {
        perror("Get a list of interface addresses failed");
        close(sock_fd);
        return "";
    }

    interface_num = ifc.ifc_len / sizeof(struct ifreq);
    printf("The number of interfaces is %d\n", interface_num);

    while(interface_num--)
    {
        printf("Net device: %s\n", buf[interface_num].ifr_name);

        if(ioctl(sock_fd, SIOCGIFFLAGS, (char *)&buf[interface_num]) < 0)
        {
            perror("Get the active flag word of the device");
            close(sock_fd);
            break;
        }

        if(buf[interface_num].ifr_flags & IFF_PROMISC)
            printf("Interface is in promiscuous mode\n");

        if(buf[interface_num].ifr_flags & IFF_UP)
            printf("Interface is running\n");
        else
            printf("Interface is not running\n");

        if(ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[interface_num]) < 0)
        {
            perror("Get interface address failed");
            close(sock_fd);
            break;
        }

        szIp = inet_ntoa(((struct sockaddr_in*)(&buf[interface_num].ifr_addr))->sin_addr);

        close(sock_fd);

        return szIp;
    }

    close(sock_fd);

    return szIp;
}

std::string GetIps(MAP_COMMON & mip)
{
    std::string szIp;
    int sock_fd;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    int interface_num;

    if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror("Create socket failed");

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_req = buf;
    if(ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc) < 0)
    {
        perror("Get a list of interface addresses failed");
        close(sock_fd);
        return "";
    }

    interface_num = ifc.ifc_len / sizeof(struct ifreq);
    printf("The number of interfaces is %d\n", interface_num);

    while(interface_num--)
    {
        printf("Net device: %s\n", buf[interface_num].ifr_name);

        if(ioctl(sock_fd, SIOCGIFFLAGS, (char *)&buf[interface_num]) < 0)
        {
            perror("Get the active flag word of the device");
            close(sock_fd);
            break;
        }

        if(buf[interface_num].ifr_flags & IFF_PROMISC)
            printf("Interface is in promiscuous mode\n");

        if(buf[interface_num].ifr_flags & IFF_UP)
            printf("Interface is running\n");
        else
            printf("Interface is not running\n");

        if(ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[interface_num]) < 0)
        {
            perror("Get interface address failed");
            close(sock_fd);
            break;
        }

        szIp = inet_ntoa(((struct sockaddr_in*)(&buf[interface_num].ifr_addr))->sin_addr);

        if(szIp.compare("127.0.0.1") != 0 && szIp.compare("0.0.0.0") != 0)
        {
            mip.insert(MAP_COMMON::value_type(szIp, szIp));
        }
    }

    close(sock_fd);

    return szIp;
}

int   EncryptMode(int flag, std::string szKey, ST_ENCRYPT& encrypt)
{
    if(flag < 7)
    {
        encrypt.mode = NEC;
        encrypt.szKey = "";
    }
    else if(flag < 13)
    {
        szKey.erase(12);
        szKey.erase(0, 7);
        encrypt.mode = AES;
        encrypt.szKey = szKey;
    }
    else if(flag < 19)
    {
        szKey.erase(18);
        szKey.erase(0, 13);
        encrypt.mode = DES;
        encrypt.szKey = szKey;
    }
    else if(flag < 25)
    {
        szKey.erase(24);
        szKey.erase(0, 19);
        encrypt.mode = RC4;
        encrypt.szKey = szKey;
    }
    else if(flag < 31)
    {
        szKey.erase(30);
        szKey.erase(0, 25);
        encrypt.mode = SM4;
        encrypt.szKey = szKey;
    }

    return 0;
}

std::string GenerteKey()
{
    int length = 33;
    int flag, i;  
    char* string;  
    srand((unsigned) time(NULL ));  
    if ((string = (char*) malloc(length)) == NULL )  
    {  
        return "kjsdijsioiejdjsiedjjd1245803hejd" ;  
    }  

    for (i = 0; i < length - 1; i++)  
    {  
        flag = rand() % 3;  
        switch (flag)  
        {  
            case 0:  
                string[i] = 'A' + rand() % 26;  
                break;  
            case 1:  
                string[i] = 'a' + rand() % 26;  
                break;  
            case 2:  
                string[i] = '0' + rand() % 10;  
                break;  
            default:  
                string[i] = 'x';  
                break;  
        }  
    }  
    string[length - 1] = '\0';  

    std::string szKey = string;

    free(string);

    return szKey;  
}

unsigned int GetRandomInteger(int low, int up)
{
    unsigned int uiResult;

    if (low > up)
    {
        int temp = low;
        low = up;
        up = temp;
    }

    uiResult = (rand() % (up - low + 1)) + low;

    return uiResult;
}

