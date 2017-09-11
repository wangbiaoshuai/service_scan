#ifndef _COMMON_H
#define _COMMON_H

// typedef unsigned long DWORD,*PDWORD,*LPDWORD;
typedef unsigned char BYTE, *PBYTE, *LPBYTE; 
typedef unsigned short WORD, *PWORD, *LPWORD;

typedef struct _CEMS_NET_HEAD
{
	unsigned int dwFlag; //_edp
	unsigned int dwVersion; //0x01010101
	unsigned int dwDataSize; //数据包大小
	unsigned int dwCrc; //数据包的crc，数据包中包含自增msgcode，防止篡改发包
	BYTE szSessionId[16]; //GUID
	unsigned int dwMsgCode; //0x00000001自增，防止伪造发包
	unsigned int dwMaxCode; //数据包业务maxCode，
	unsigned int dwMinCode; //数据包业务minCode， 
	WORD wHeadSize; //sizeof(CEMS_TCP_HEAD)
	WORD wType; //数据包是zip格式的
	WORD wCount; //3
	WORD wIndex; //1,2,3
	BYTE szAreaId[32]; //区域ID
} CEMS_NET_HEAD, *PCEMS_NET_HEAD;

#endif