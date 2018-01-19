#include "CRC32.h"


/*********************************************************************
*	�������ƣ�	CalcCrc32
*	����������	���ֽ�CRC32У���㷨 ��������
*	����˵����	const unsigned char byte[����ĵ��ֽ�],
*				unsigned int *dwCrc32[�ϴε��ֽ�У����]
*	�� �� ֵ��  
*********************************************************************/
inline void CalcCrc32(const unsigned char byte,unsigned int *dwCrc32)
{
	*dwCrc32 = ((*dwCrc32)>> 8) ^ s_arrdwCrc32Table[(byte)^((*dwCrc32)&0x000000FF)];
}

/*********************************************************************
*	�������ƣ�	StringCrc32
*	����������	�ļ�����CRC32У��
*	����˵����	const unsigned char * buf[��������], 
*				unsigned int *dwCrc32[CRC32������],
*				int DataLen[�ļ�����У�鳤��]
*	�� �� ֵ��  
*********************************************************************/
int StringCrc32(const unsigned char * buf, unsigned int *dwCrc32, int DataLen)
{
	int i=0;

	*dwCrc32 = 0xFFFFFFFF;//��ʼ��CRCУ���� Ĭ��ֵ
  
    for(i=0;i<DataLen;i++) 
	{
		CalcCrc32((unsigned char)*buf, dwCrc32);
		buf++;
	} 

	*dwCrc32 = ~ *dwCrc32;
	return 0;
}