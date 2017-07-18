#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <dlfcn.h>
#include "common_function.h"
#include "gen_algorithm.h"


#define MAXBUFSIZE         260
#define SO_ZLIB		   "/libMZlib.so"
#define SO_CRYPT	   "/libMCrypt.so"

unsigned char iv[16] = {0x7d, 0xac, 0x31, 0x6, 0xaf, 0x9f, 0x69, 0xad, 0x3c, 0x47, 0xe9, 0x94, 0x9d, 0xd4, 0x6b, 0xd};

#define HEADER_LEN   50
typedef unsigned long uLong;

CGenAlgori::CGenAlgori():m_zlibHandle(NULL), m_cryptHandle(NULL)
{
    load();
}

CGenAlgori::~CGenAlgori()
{
    unload();
}

void CGenAlgori::load()
{
    std::string szZlibPath;
    std::string szCryptPath;

    std::string szPath = GetCurrentPath();

    size_t pos = szPath.rfind("/");
    if(pos != std::string::npos)
    {
        szPath.erase(pos);
    }
    szPath += "/lib";

    szZlibPath = szPath;
    szCryptPath = szPath;	

    szZlibPath += SO_ZLIB;
    szCryptPath += SO_CRYPT;

    m_zlibHandle = dlopen(szZlibPath.c_str(), RTLD_NOW);
    if(!m_zlibHandle)
    {
        printf("load so fail: %s\n", dlerror());
        printf("path = %s\n", szZlibPath.c_str());

    }
    else
    {
        m_pdeflate_compress  = (pdeflate_compress)dlsym(m_zlibHandle, "deflate_compress");
        m_pdeflate_uncompress = (pdeflate_uncompress)dlsym(m_zlibHandle, "deflate_uncompress");

        m_pgzip_compress = (pgzip_compress)dlsym(m_zlibHandle, "gzip_compress");
        m_pgzip_uncompress = (pgzip_uncompress)dlsym(m_zlibHandle, "gzip_uncompress");
    }

    m_cryptHandle = dlopen(szCryptPath.c_str(), RTLD_NOW);
    if(!m_cryptHandle)
    {
        printf("load so fail: %s\n", dlerror());
        printf("path = %s\n", szCryptPath.c_str());
    }
    else
    {
        m_pCEMS_Encrypt_Ex = (pCEMS_Encrypt_Ex)dlsym(m_cryptHandle, "CEMS_Encrypt_Ex");
        m_pCEMS_Decrypt_Ex = (pCEMS_Decrypt_Ex)dlsym(m_cryptHandle, "CEMS_Decrypt_Ex");
    }	
}

void CGenAlgori::unload()
{
    if(m_zlibHandle)
    {
        dlclose(m_zlibHandle);
        m_zlibHandle = NULL;
    }

    if(m_cryptHandle)
    {
        dlclose(m_cryptHandle);
        m_cryptHandle = NULL;
    }
}

std::string CGenAlgori::EnCrypt(std::string strIn, unsigned int type, std::string password)
{
    unsigned char ivTemp[16];
    memcpy(ivTemp, iv, 16);

    std::string szBase;

    char * pType = NULL;
    if(type == DES)
    {
        pType = (char*)"DES";
    }
    else if(type == AES)
    {
        pType = (char*)"AES";
    }
    else if(type == RC4)
    {
        pType = (char*)"RC4";
    }
    else if(type == SM4)
    {
        pType = (char*)"SM4";
    }
    else //if(type == NEC)
    {
        return strIn;
    }

    szBase  = m_pCEMS_Encrypt_Ex(pType, strIn, password, ivTemp);

    return szBase;
}

std::string CGenAlgori::DeCrypt(std::string strIn, unsigned int type, std::string password)
{
    unsigned char ivTemp[16];
    memcpy(ivTemp, iv, 16);

    std::string szOutPut;

    char * pType = NULL;
    if(type == DES)
    {
        pType = (char*)"DES";
    }
    else if(type == AES)
    {
        pType = (char*)"AES";
    }
    else if(type == RC4)
    {
        pType = (char*)"RC4";
    }
    else if(type == SM4)
    {
        pType = (char*)"SM4";
    }
    else // if(type == NEC)
    {
        return strIn;
    }

    szOutPut  = m_pCEMS_Decrypt_Ex(pType, strIn, password, ivTemp);

    return szOutPut;
}

std::string CGenAlgori::Compress(std::string strIn, unsigned int type)
{
    if(m_pdeflate_compress == NULL || m_pgzip_compress == NULL)
    {
        return "";
    }

    char * bufout = NULL; 
    int CompressedLen = strIn.length() + HEADER_LEN;
    int lenout = 0;

    std::string szBufOut;

    bufout = new char[CompressedLen + 1];
    bzero(bufout, CompressedLen + 1);

    if(type == DEFLATE)
    {
        lenout = m_pdeflate_compress((unsigned char*)strIn.c_str(), strIn.size(), (unsigned char*)bufout, CompressedLen);
    }
    else if(type == GZIP)
    {
        lenout = m_pgzip_compress((unsigned char*)strIn.c_str(), strIn.size(), (unsigned char*)bufout, CompressedLen);
    }
    else //if(type == DEFAULT)
    {
        delete []bufout;
        return strIn;
    }

    if(lenout < 0)
    {
        printf("compress fail error code = %d, type = %d\n", lenout, type);			
    }
    else
    {
        szBufOut.assign(bufout, lenout);
    }

    delete []bufout;

    return szBufOut;
}

std::string CGenAlgori::UnCompress(std::string strIn, unsigned int type)
{
    std::string szBufOut;
    char* bufout = NULL;
    int mul = 2;
    int lenout = 0; 
BEGIN:
    uLong lensrc = strIn.size() * mul;

    bufout = new char[lensrc + 1];
    bzero(bufout, lensrc + 1);

    if(type == DEFLATE)
    {
        lenout = m_pdeflate_uncompress((unsigned char*)strIn.data(), strIn.size(), (unsigned char*)bufout, lensrc);
    }
    else if(type == GZIP)
    {
        lenout = m_pgzip_uncompress((unsigned char*)strIn.data(), strIn.size(), (unsigned char*)bufout, lensrc);
    }
    else //if(type == DEFAULT)
    {
        delete []bufout;
        return strIn;
    }


    if(lenout == -5) /*解压空间不足*/
    {
        delete []bufout; bufout = NULL;
        mul *= 2;

        goto BEGIN;
    }

    szBufOut.assign(bufout, lensrc);	

    delete []bufout;

    return szBufOut;
}

/*int main(int argc, char* argv[])
  {
  CGenAlgori cga;

  std::string szPassWord = "123456";
  std::string szSrc = "这是一个测试字符串1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";

  printf("原始长度: %d\n", szSrc.size());

  std::string szEnc = cga.EnCrypt(szSrc, AES, szPassWord);
  std::string szDec = cga.DeCrypt(szEnc, AES, szPassWord);

  printf("原文 = %s\n", szDec.c_str());

  std::string szCompress = cga.Compress(szDec, DEFLATE);
  std::string szUnCompress = cga.UnCompress(szCompress, DEFLATE);

  printf("压缩后长度: %d\n", szCompress.size());
  printf("原文 = %s\n", szUnCompress.c_str());

  return 0;
  }*/

