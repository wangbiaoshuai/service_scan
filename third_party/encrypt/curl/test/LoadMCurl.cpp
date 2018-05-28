#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

#define LIB_CURL_SO       "./libMCurl.so"

typedef bool (*pPostRequest)(const char*, const char*, char*, int &);

extern "C" bool  PostRequest(const char* szUrl, unsigned char* szPostData, int dataLen,  char* pbuffer, int & bufSize);

void TestStcLib()
{
   const char * purl = "https://www.baidu.com";
   const char * pszPostData = NULL;
   int  size = 1024*100;
   char buffer[1024*100] = {0};

   bool bRet = PostRequest(purl, (unsigned char*)pszPostData, 0,  buffer, size);

   printf(buffer);	
}

int main()
{
	TestStcLib();
	return 0;

	const char * purl = "https://218.26.4.141/por/login_psw.csp";
	const char * pszPostData = NULL;
	
	int  size = 1024;
	int  ssize = 1024;
	char buffer[1024] = {0};

	void * handle = NULL;
	pPostRequest PostRequest = NULL;

	handle  = dlopen(LIB_CURL_SO, RTLD_NOW);

	if(handle)
	{
		PostRequest = (pPostRequest)dlsym(handle, "PostRequest");
		if(!PostRequest)
		{
			char * perror = dlerror();
			printf("error = %s\n", perror);
		}

		bool bRet = PostRequest(purl, pszPostData, buffer, size);
		if(bRet)
		{
			printf("%s\n", buffer);			
		}
		else
		{
			if(size > ssize)
			{
				char* pbuffer = new char[size + 1];
				bzero(pbuffer, size + 1);

				bRet = PostRequest(purl, pszPostData, pbuffer, size);
				if(bRet)
				{
					printf("%s\n", pbuffer);
				}
				else
				{
					printf("failed\n");
				}

				delete []pbuffer;
			}
			else
			{
				printf("failed\n");
			}
		}

		dlclose(handle);
	}
	else
	{
		 char * perror = dlerror();
                 printf("error = %s\n", perror);
	}	
	return 0;
}
