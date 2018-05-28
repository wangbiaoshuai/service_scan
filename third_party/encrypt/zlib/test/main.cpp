#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string>

#include "Base64.h"

#define HEADER_LEN   50
typedef unsigned long uLong;

typedef int (* pdeflate_compress)(unsigned char *, int, unsigned char *, int);
typedef int (* pdeflate_uncompress)(unsigned char *, int, unsigned char *, int);

typedef int (* pgzip_compress)(unsigned char *, int, unsigned char *, int);
typedef int (* pgzip_uncompress)(unsigned char *, int, unsigned char *, int);

int main(int argc, char* argv[])
{
        if(argc != 3)
        {
                printf("参数格式错误\n");

                printf("useage:\n");
                printf("zlib  /deflate\t\"汉字123abc~!@#$%^&*(\"\n");
                printf("zlib  /gzip \t\"汉字123abc~!@#$%^&*(\"\n");
                return -1;
        }

	void * handle = NULL;
        char * pszPath = (char*)"/home/vrv/Test/so/zlib/libMZlib.so";

	handle = dlopen(pszPath, RTLD_NOW);
	if(!handle)
	{
		printf("load so fail: %s\n", dlerror());
	}	

	pdeflate_compress deflate_compress = (pdeflate_compress)dlsym(handle, "deflate_compress");
	pdeflate_uncompress deflate_uncompress = (pdeflate_uncompress)dlsym(handle, "deflate_uncompress");

	pgzip_compress gzip_compress = (pgzip_compress)dlsym(handle, "gzip_compress");
	pgzip_uncompress gzip_uncompress = (pgzip_uncompress)dlsym(handle, "gzip_uncompress");

        static int iCount = 1;
begin:
        iCount++;
        printf("压缩内容: %s\n", argv[2]);

        int result;
        std::string szSrc = argv[2];

        char *bufin;
        bufin = (char*)szSrc.c_str();

         char *bufout = NULL;
         char *bufout2 = NULL;

         uLong lenout;
         int   lensrc = strlen(bufin);

         char *pType = argv[1];

         printf("压缩方式: %s\n", pType);
         printf("压缩前：%d 字节\n", strlen(bufin));

         int CompressedLen = lensrc + HEADER_LEN;

         bufout = new char[CompressedLen + 1];
         bzero(bufout, CompressedLen + 1);

	 if(strcmp(pType, "/deflate") == 0)
         {
                lenout = deflate_compress((unsigned char*)bufin, lensrc, (unsigned char*)bufout, CompressedLen);
         }
         else if(strcmp(pType, "/gzip") == 0)
         {
                lenout = gzip_compress((unsigned char*)bufin, lensrc, (unsigned char*)bufout, CompressedLen);
         }

         std::string szBaseGzip;
         CBase64::Encode((unsigned char*)bufout, (long)lenout, szBaseGzip);

         printf("压缩后: base64 ： %s\n",  szBaseGzip.c_str());

         printf("压缩后：%d 字节\n",  lenout);

#ifndef MY_DEBUG
         for(int i = 0; i < lenout; i++)
         {
                 printf("0x%02x ", (unsigned char)bufout[i]);
                 if((i + 1) % 8 == 0)
                 {
                        printf("\n");
                 }
         }
         printf("\n");
#endif

         bzero(bufout, lenout);
         CBase64::Decode(szBaseGzip, (unsigned char*)bufout, &lenout);

         bufout2 = new char[lensrc + 1];
         bzero(bufout2, lensrc + 1);

	 bufout2 = new char[lensrc + 1];
         bzero(bufout2, lensrc + 1);

         if(strcmp(pType, "/deflate") == 0)
         {
                 deflate_uncompress((unsigned char*)bufout, lenout, (unsigned char*)bufout2, lensrc);
         }
         else if(strcmp(pType, "/gzip") == 0)
         {
                 gzip_uncompress((unsigned char*)bufout, lenout, (unsigned char*)bufout2, lensrc);
         }

         printf("解压缩后: %s\n", bufout2);

         delete []bufout;
         delete []bufout2;

         if(iCount > 2 )
                 goto end;

         goto begin;

end:
	dlclose(handle);

        return 0;       
}
