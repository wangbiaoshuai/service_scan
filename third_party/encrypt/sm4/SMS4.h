#ifndef __SMS4_H__2015_03_24__
#define __SMS4_H__2015_03_24__

typedef struct _SMS4_CTX
{
	unsigned long Bit;
	unsigned long Offset;
	unsigned long Extandkey[32];
	unsigned char iv[16];
}SMS4_CTX, *PSMS4_CTX;

void  SMS4_SetKey(PSMS4_CTX f_Context, const unsigned char *key, const unsigned char *iv, unsigned long ivoffset);

void  SMS4_ECB_Encrypt(PSMS4_CTX f_Context, const unsigned char *input, unsigned char *output);

void  SMS4_ECB_Decrypt(PSMS4_CTX f_Context, const unsigned char *input, unsigned char *output);

unsigned long  SMS4_CBC_Encrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output);

unsigned long  SMS4_CBC_Decrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output);

unsigned long  SMS4_CFB_Encrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output);

unsigned long SMS4_CFB_Decrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output);

unsigned long SMS4_OFB_Encrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output);

unsigned long SMS4_OFB_Decrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output);

#endif // __SMS4_H__2015_03_24__
