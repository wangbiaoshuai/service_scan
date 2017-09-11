#include "CRC32.h"


/*********************************************************************
*	函数名称：	CalcCrc32
*	函数描述：	单字节CRC32校验算法 内链函数
*	参数说明：	const unsigned char byte[传入的单字节],
*				unsigned int *dwCrc32[上次单字节校验结果]
*	返 回 值：  
*********************************************************************/
inline void CalcCrc32(const unsigned char byte,unsigned int *dwCrc32)
{
	*dwCrc32 = ((*dwCrc32)>> 8) ^ s_arrdwCrc32Table[(byte)^((*dwCrc32)&0x000000FF)];
}

/*********************************************************************
*	函数名称：	StringCrc32
*	函数描述：	文件数据CRC32校验
*	参数说明：	const unsigned char * buf[传入数据], 
*				unsigned int *dwCrc32[CRC32返回码],
*				int DataLen[文件数据校验长度]
*	返 回 值：  
*********************************************************************/
int StringCrc32(const unsigned char * buf, unsigned int *dwCrc32, int DataLen)
{
	int i=0;

	*dwCrc32 = 0xFFFFFFFF;//初始化CRC校验码 默认值
  
    for(i=0;i<DataLen;i++) 
	{
		CalcCrc32((unsigned char)*buf, dwCrc32);
		buf++;
	} 

	*dwCrc32 = ~ *dwCrc32;
	return 0;
}