#include <unistd.h>
#include <string.h>

#include "SMS4.h"

#define SMS4_ROUND            32

static UINT SMS4_FK[4] = 
{
	0xA3B1BAC6,0x56AA3350,0x677D9197,0xB27022DC
};

static UINT SMS4_CK[SMS4_ROUND] = 
{
	0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
	0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
	0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
	0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
	0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
	0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
	0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
	0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
};

static unsigned char SMS4_SBOX[256] =
{
	0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
	0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
	0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
	0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
	0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
	0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
	0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
	0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
	0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
	0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
	0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
	0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
	0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
	0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
	0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
	0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48
};

#define SMS4_ROL(x,y)    ((x)<<(y) |    (x)>>(32-(y)))
//---------------------------------------------------------------------------
unsigned int SMS4_T1(unsigned int    dwA)
{
	unsigned char    a0[4]={0};
	unsigned char    b0[4]={0};
	unsigned int    dwB=0;
	unsigned int    dwC=0;
	int                i=0;

	for (i = 0; i < 4; i++)
	{
		a0[i] = (unsigned char)((dwA>>(i*8)) & 0xff);
		b0[i] = SMS4_SBOX[a0[i]];
		dwB  |= (b0[i]<<(i*8));
	}
	
	dwC=dwB^SMS4_ROL(dwB,2)^SMS4_ROL(dwB,10)^SMS4_ROL(dwB,18)^SMS4_ROL(dwB,24);

	return dwC;
}
//---------------------------------------------------------------------------
unsigned int SMS4_T2(unsigned int    dwA)
{
	unsigned char    a0[4]={0};
	unsigned char    b0[4]={0};
	unsigned int    dwB=0;
	unsigned int    dwC=0;
	int				 i	=0;

	for (i = 0; i < 4; i++)
	{
		a0[i] = (unsigned char)((dwA>>(i*8)) & 0xff);
		b0[i] = SMS4_SBOX[a0[i]];
		dwB  |= (b0[i]<<(i*8));
	}

	dwC=dwB^SMS4_ROL(dwB,13)^SMS4_ROL(dwB,23);

	return dwC;
}
//---------------------------------------------------------------------------
/* MK[4] is the Encrypt Key, rk[32] is Round Key */
void SMS4_Key_Expansion(unsigned int MK[],    unsigned int rk[])
{
	unsigned int K[4] = { 0 };
	int			  i    = 0;
	
	for (i = 0; i < 4; i++)
	{
		K[i] = MK[i] ^ SMS4_FK[i];
	}
	
	for (i= 0; i < SMS4_ROUND; i++)
	{
		K[i%4] ^= SMS4_T2(K[(i+1)%4]^K[(i+2)%4]^K[(i+3)%4]^SMS4_CK[i]);
		rk[i]   = K[i%4];
	}
}
//---------------------------------------------------------------------------
/* X[4] is PlainText, rk[32] is round Key, Y[4] is CipherText */
void SMS4_ECB_Encryption_Core(unsigned int X[], unsigned int rk[], unsigned int Y[])
{
	unsigned int    tempX[4]	= { 0 };
	int              i			= 0;

	for (i = 0; i < 4; i++)
	{
		tempX[i] = X[i];
	}
	
	for (i = 0; i < SMS4_ROUND; i++)
	{
		tempX[i%4] ^= SMS4_T1(tempX[(i+1)%4]^tempX[(i+2)%4]^tempX[(i+3)%4]^rk[i]);
	}
	
	for (i = 0; i < 4; i++)
	{
		Y[i] = tempX[3-i];
	}
}
//---------------------------------------------------------------------------
/* X[4] is PlainText, rk[32] is round Key, Y[4] is CipherText */
void SMS4_ECB_Decryption_Core(unsigned int X[], unsigned int rk[], unsigned int Y[])
{
	unsigned int    tempX[4] = { 0 };
	int              i		  = 0;

	for (i = 0; i < 4; i++)
	{
		tempX[i] = X[i];
	}
	
	for (i = 0; i < SMS4_ROUND; i++)
	{
		tempX[i%4] ^= SMS4_T1(tempX[(i+1)%4]^tempX[(i+2)%4]^tempX[(i+3)%4]^rk[(31-i)]);
	}
	
	for (i = 0; i < 4; i++)
	{
		Y[i] = tempX[3-i];
	}
}
//---------------------------------------------------------------------------
void SMS4_convert_to_network_order(unsigned int* src,unsigned int* dst,int count)
{
	int i=0;
	
	for ( ; i<count; i++ )
	{
		unsigned char* ps = (unsigned char*)(src+i);
		unsigned char* pd = (unsigned char*)(dst+i);
		
		pd[0] = ps[3];
		pd[1] = ps[2];
		pd[2] = ps[1];
		pd[3] = ps[0];
	}
}
//---------------------------------------------------------------------------
void SMS4_convert_to_host_order(unsigned int* src,unsigned int* dst,int count)
{
	SMS4_convert_to_network_order(src,dst,count);
}

void  SMS4_SetKey(PSMS4_CTX f_Context, const unsigned char *key, const unsigned char *iv, unsigned int ivoffset)
{
	unsigned int _ky[4];

	f_Context->Bit		= 128;
	f_Context->Offset	= ivoffset;

	if(iv)
	{
		memcpy(f_Context->iv, iv, 16);
	}

	SMS4_convert_to_network_order((unsigned int*)key, _ky, 4);

	SMS4_Key_Expansion(_ky, f_Context->Extandkey);
}

void  SMS4_ECB_Encrypt(PSMS4_CTX f_Context, const unsigned char *input, unsigned char *output)
{
	unsigned int _pt[4];
	unsigned int _ct[4];

	SMS4_convert_to_network_order((unsigned int*)input,_pt,4);
	

	SMS4_ECB_Encryption_Core(_pt,f_Context->Extandkey,_ct);
	
	SMS4_convert_to_host_order(_ct,(unsigned int*)output,4);
}

void  SMS4_ECB_Decrypt(PSMS4_CTX f_Context, const unsigned char *input, unsigned char *output)
{
	unsigned int _ct[4];
	unsigned int _pt[4];
	
	SMS4_convert_to_network_order((unsigned int*)input,_ct,4);
	
	SMS4_ECB_Decryption_Core(_ct,f_Context->Extandkey,_pt);
	
	SMS4_convert_to_host_order(_pt,(unsigned int*)output,4);
}

UINT SMS4_CBC_Encrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output)
{
	size_t	i, j, n = Length / 16;
	
	for(i = 0; i < n; i++)
	{
		for(j = 0; j < 16; j++)
		{
			output[j] = input[j] ^ f_Context->iv[j];
		}
		
		SMS4_ECB_Encrypt(f_Context, output, output);
		
		memcpy( f_Context->iv, output, 16 );
		
		input += 16; output += 16;
	}
	
	return (UINT)(n * 16);
}

UINT  SMS4_CBC_Decrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output)
{
	size_t			i, j, n;
	unsigned char	temp[16];
	
	n = Length / 16;
	
	for(i = 0; i < n; i++)
	{
		memcpy( temp, input, 16 );
		
		SMS4_ECB_Decrypt(f_Context, input, output);
		
		for(j = 0; j < 16; j++)
		{
			output[j] ^= f_Context->iv[j];
		}
		
		memcpy( f_Context->iv, temp, 16 );
		
		input += 16; output += 16;
	}
	
	return (UINT)(n * 16);
}

UINT SMS4_CFB_Encrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output)
{
	size_t i, n;

	for(i = 0; i < Length; i++, input++, output++)
	{
		if( (n = (f_Context->Offset % 16)) == 0 )
		{
			SMS4_ECB_Encrypt( f_Context, f_Context->iv, f_Context->iv );
		}

		f_Context->iv[n]  = *output = f_Context->iv[n] ^ *input;
		f_Context->Offset = (UINT)(n + 1);
	}

	return (UINT)Length;
}

UINT SMS4_CFB_Decrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output)
{
	size_t i, n; unsigned char c;

	for(i = 0; i < Length; i++, input++, output++)
	{
		if( (n = (f_Context->Offset % 16)) == 0 )
		{
			SMS4_ECB_Encrypt( f_Context, f_Context->iv, f_Context->iv );
		}

		c       = *input;
		*output = *input ^ f_Context->iv[n];

		f_Context->iv[n]  = c;
		f_Context->Offset = (UINT)(n + 1);
	}

	return (UINT)Length;
}

UINT SMS4_OFB_Encrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output)
{
	size_t i, n;
	
	for(i = 0; i < Length; i++, input++, output++)
	{
		if( (n = (f_Context->Offset % 16)) == 0 )
		{
			SMS4_ECB_Encrypt( f_Context, f_Context->iv, f_Context->iv );
		}
		
		*output			  = f_Context->iv[n] ^ *input;
		f_Context->Offset = (UINT)(n + 1);
	}
	
	return (UINT)Length;
}

UINT SMS4_OFB_Decrypt(PSMS4_CTX f_Context, const unsigned char *input, size_t Length, unsigned char *output)
{
	return SMS4_OFB_Encrypt(f_Context, input, Length, output);
}

/*SM4*/
extern "C" UINT SMS4_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
	unsigned char ivTemp[16] = {0};
	memcpy(ivTemp, iv, 16);

	SMS4_CTX ctx;
	SMS4_SetKey(&ctx, key, ivTemp, 0);

	UINT  uLen = SMS4_CFB_Encrypt(&ctx, plaintext, plaintext_len, ciphertext);
	
	return uLen;
}


extern "C" UINT SMS4_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
	unsigned char ivTemp[16];
	memcpy(ivTemp, iv, 16);

	SMS4_CTX ctx;
	SMS4_SetKey(&ctx, key, ivTemp, 0);

	UINT  uLen = SMS4_CFB_Decrypt(&ctx, ciphertext, ciphertext_len, plaintext);

	return uLen;
}
