#ifndef _M_CONV
#define _M_CONV

#include <iconv.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>

int gbk2utf8(char *utfStr, const char *srcStr, int maxUtfStrlen);

class CodeConvert
{
public:
	static std::string u2g(std::string szUtf8);
	static std::string g2u(std::string szGbk);
	static int convert(iconv_t  cd, char *inbuf,int inlen,char *outbuf,int outlen);
};

#endif
