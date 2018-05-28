#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

#include <string>

#define HEX_PRINT(T, L)  //printf("密文: \n"); for(int i = 0; i < T, i++) printf("0x%X ", L[i]); printf("\n");

typedef int (*pCEMS_encrypt)(char*, unsigned char *, int , unsigned char *, unsigned char *, int, unsigned char *);
typedef int (*pCEMS_decrypt)(char*, unsigned char *, int , unsigned char *, unsigned char *, int, unsigned char *);

typedef int (*pSM4_encrypt)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *);
typedef int (*pSM4_decrypt)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *);

typedef std::string (*pCEMS_Encrypt_Ex)(std::string, std::string, std::string, unsigned char*);
typedef std::string (*pCEMS_Decrypt_Ex)(std::string, std::string, std::string, unsigned char*);


unsigned char iv[16] = {0x7d, 0xac, 0x31, 0x6, 0xaf, 0x9f, 0x69, 0xad, 0x3c, 0x47, 0xe9, 0x94, 0x9d, 0xd4, 0x6b, 0xd};

int main()
{
	printf("unsigned long size is %d\n", sizeof(unsigned int));
	void * handle = NULL;
	char * pszPath = (char*)"/home/vrv/cems/so/crypto/libMCrypt.so";	

	std::string pKey = (char*)"_EDP_EDP_EDP_EDP";

	std::string pcontent = (char*)"汉字jisjd1234567812345678";
	std::string  pType = (char*)"DES";

	unsigned char* pOutCiper = NULL;
	unsigned char* pOutText = NULL;

	handle = dlopen(pszPath, RTLD_NOW);
	if(!handle)
	{
		printf("load so fail : %s\n", dlerror());
	}

	//pCEMS_encrypt CEMS_encrypt = (pCEMS_encrypt)dlsym(handle, "CEMS_encrypt");
	//pCEMS_decrypt CEMS_decrypt = (pCEMS_decrypt)dlsym(handle, "CEMS_decrypt");

	pCEMS_Encrypt_Ex CEMS_Encrypt_Ex = (pCEMS_Encrypt_Ex)dlsym(handle, "CEMS_Encrypt_Ex");
	pCEMS_Decrypt_Ex CEMS_Decrypt_Ex = (pCEMS_Decrypt_Ex)dlsym(handle, "CEMS_Decrypt_Ex");

         int len = pcontent.length();
	 int keylen = pKey.length();

         //pOutCiper = new unsigned char[len];
         //pOutText = new unsigned char[len + 1];

         //memset(pOutText, 0, len+1);
	 printf("加密类型: %s\n", (char*)pType.c_str());
	 printf("明文:%s\n", (char*)pcontent.c_str());
	
	//CEMS_encrypt((char*)pType.c_str(), (unsigned char*)pcontent.c_str(), len, pOutCiper, (unsigned char*)pKey.c_str(), keylen, iv);
	//CEMS_decrypt((char*)pType.c_str(), pOutCiper, len, pOutText, (unsigned char*)pKey.c_str(), keylen, iv);

	//printf("解密后: %s\n", pOutText);

        //delete []pOutCiper; pOutCiper = NULL;
        //delete []pOutText; pOutText = NULL;
	
	unsigned char ivTemp[16];
	memcpy(ivTemp, iv, 16);
	std::string szBase = CEMS_Encrypt_Ex(pType, pcontent, pKey, ivTemp);
	//printf("base64:%s\n", (char*)szBase.c_str());

	memcpy(ivTemp, iv, 16);
	std::string szOutPut = CEMS_Decrypt_Ex(pType, szBase, pKey, iv);

	printf("解密后:%s\n", (char*)szOutPut.c_str());

	dlclose(handle);

	return 0;
}
