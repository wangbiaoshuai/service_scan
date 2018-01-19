
#include "conv.h"

int gbk2utf8(char *utfStr, const char *srcStr, int maxUtfStrlen)  
{  
    if(NULL == srcStr)  
    {    
        return -1;  
    }  
  
    if(NULL == setlocale(LC_ALL, "zh_CN.gbk"))
    {    
        return -1;  
    }  
  
    int unicodeLen = mbstowcs(NULL, srcStr, 0);
    if(unicodeLen <= 0)  
    {    
        return -1;  
    }  
    wchar_t *unicodeStr = (wchar_t *)calloc(sizeof(wchar_t), unicodeLen+1);  
    mbstowcs(unicodeStr,srcStr,strlen(srcStr));
     
    int utfLen; 
    if(NULL == setlocale(LC_ALL,"zh_CN.utf8"))
    {   
	utfLen =  -1;
	goto END;
      
    }  
    utfLen = wcstombs(NULL, unicodeStr, 0);
    if(utfLen <= 0)  
    {
	utfLen =  -1;   
	goto END;
    
    }  
    else if(utfLen >= maxUtfStrlen)
    {  
        goto END;  
    }  

    wcstombs(utfStr, unicodeStr, utfLen);  
    utfStr[utfLen] = 0;

END:
    free(unicodeStr);  
    return utfLen;  
}  

std::string CodeConvert::u2g(std::string szUtf8)
{
	std::string szRet;
	if(szUtf8.empty())
	{
		return "";
	}

	iconv_t ic = iconv_open("gbk", "utf-8");
	if(ic != (iconv_t)-1)
	{	
		int len = szUtf8.length();
		char * out = new char[len];
		convert(ic, (char*)szUtf8.c_str(), len, out, len);
		szRet = out;
		iconv_close(ic);
		delete []out;
	}
	else
	{
		szRet = szUtf8; 
	}

	return szRet;
}

std::string CodeConvert::g2u(std::string szGbk)
{
	std::string szRet;
	int len = gbk2utf8(NULL, szGbk.c_str(), 0);
	if(len != -1)
	{
		char * pbuffer = new char[len + 1];
		memset(pbuffer, 0, len + 1);
	
		if(gbk2utf8(pbuffer, szGbk.c_str(), len + 1) == -1)
		{
			szRet = szGbk;
		}
	
		szRet = pbuffer;

		delete []pbuffer;
	}
	else
	{
		szRet = szGbk;
	}
	
	return szRet;
}

int CodeConvert::convert(iconv_t  cd, char *inbuf,int inlen,char *outbuf,int outlen) 
{
	char **pin = &inbuf;
	char **pout = &outbuf;

	memset(outbuf, 0, outlen);
	return iconv(cd, pin, (size_t *)&inlen, pout, (size_t *)&outlen);
}
