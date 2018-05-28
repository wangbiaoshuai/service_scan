#include <unistd.h>
#include <string>
#include <string.h>
#include <time.h>
#define   CURL_STATICLIB  1

#include "curl.h"

#define  BOOL   bool
#define  TRUE   true
#define  FALSE  false

#define		CURL_MAX_REDIRECTION_COUNT		5 //s
#define		CURL_CONNET_TIMEOUT             	10*1000	
#define		CURL_READ_TIMEOUT		        300*1000
#define		CURL_OFFNET_TIMEOUT			5  //s

extern "C" BOOL  PostRequest(const char* szUrl, unsigned char* szPostData, int dataLen,  char* pbuffer, int & bufSize);

size_t  write_post_reply(void *ptr, size_t size, size_t nmemb, void* param)
{
	std::string * p = static_cast<std::string*>(param);

	if(p)
	{
		p->append((char*)ptr, nmemb*size);
	}

	return size*nmemb;
}

BOOL  PostRequest(const char* szUrl, unsigned char* szPostData, int dataLen, char* pbuffer, int & bufSize)
{
	unsigned char* pszPostTemp = NULL;
	if(szPostData == NULL)
	{
		pszPostTemp = (unsigned char*)" "; dataLen = strlen(" ");		
	}
	else
	{
		pszPostTemp = szPostData;
	}


	CURL *curl = NULL;
	//std::string szProxyAddr;
	std::string szCookie;
	std::string szPostReply;

	BOOL bRet = TRUE;
	
	CURLcode code;
	struct curl_slist *slist = NULL;

	curl = curl_easy_init();
	if(curl)
	{
		//curl 版本
		//curl_version_info_data* pdata = curl_version_info(CURLVERSION_NOW);

		if(curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1) != CURLE_OK) {bRet = FALSE; goto END;} //重定向
		if(curl_easy_setopt(curl, CURLOPT_MAXREDIRS, CURL_MAX_REDIRECTION_COUNT) != CURLE_OK) {bRet = FALSE; goto END;}//最大重定向次数

		if(curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1) != CURLE_OK)  {bRet = FALSE; goto END;}
		if(curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1) != CURLE_OK)  {bRet = FALSE; goto END;} 

		if(curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, CURL_CONNET_TIMEOUT) != CURLE_OK)  {bRet = FALSE; goto END;}

		if(curl_easy_setopt( curl, CURLOPT_COOKIEJAR, szCookie.c_str()) != CURLE_OK)  {bRet = FALSE; goto END;}

		if(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,  0) != CURLE_OK) {bRet = FALSE; goto END;}
		if(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,  0) != CURLE_OK) {bRet = FALSE; goto END;}

		/*设置代理*/
		/*if(curl_easy_setopt(curl, CURLOPT_PROXY, szProxyAddr.c_str()) != CURLE_OK){bRet = FALSE; goto END;}*/

		/*post*/
		code = curl_easy_setopt(curl, CURLOPT_POST, 1); 

		slist = curl_slist_append(slist, "Content-Type: ");
		code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

		code = curl_easy_setopt(curl, CURLOPT_URL, szUrl);
		code = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)dataLen);
		code = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pszPostTemp);

		if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &szPostReply) != CURLE_OK) {bRet = FALSE; goto END;}
		if(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_post_reply) != CURLE_OK) {bRet = FALSE; goto END;}

		if((code = curl_easy_perform(curl)) == CURLE_OK) 
		{
			if((int)szPostReply.size() > bufSize)
			{
				bufSize = szPostReply.size();
				bRet = FALSE;
			}
			else
			{
				bufSize = szPostReply.size();
				memcpy(pbuffer, szPostReply.data(), szPostReply.size());
				bRet = TRUE;
			}
		}
		else
		{
			bRet = FALSE;
		}

END:
		curl_easy_cleanup(curl);

		return bRet;
	}

	return FALSE;
}

/*int main()
{
        const char* purl = "http://192.168.0.134:8200/CEMS-C-TCP/TCPServlet";
	const char* postdata = "123";
	char pReply[1024*1024] = {0};	

	int size = 1024*1024;

	BOOL bRet = PostRequest(purl, (unsigned char*)postdata, strlen(purl), pReply, size);
	if(bRet)
	{
		printf("reply: %s\n", pReply);
	}	
	else
	{
		printf("PostRequest fail");
	}
	return 0;	
}*/	
