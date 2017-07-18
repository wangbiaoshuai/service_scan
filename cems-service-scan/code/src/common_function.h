#ifndef COMMON_FUNCTION_H_
#define COMMON_FUNCTION_H_

#include "defines.h"
void  		itoa(unsigned long val,  char *buf, unsigned radix);
bool  		CreateDirectory(const char * sPathName);
std::string 	GetCurrentPath();
std::string 	calCRC(std::string szText);
std::string  	GetConfigFilePath();
std::string  	GetCurrentIp();
std::string  	GenericUUID(std::string szKey);
std::string  	GetStrMd5Ex(char* szSrc, int iLen);
std::string 	GetIps(MAP_COMMON & mip);
void 		OutPutString(char* szContent);
int   		EncryptMode(int flag, std::string szKey, ST_ENCRYPT& encrypt);
std::string 	GenerteKey();
unsigned int    GetRandomInteger(int low, int up);

#endif // COMMON_FUNCTION_H
