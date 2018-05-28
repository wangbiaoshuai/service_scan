#include <string.h>
#include <stdlib.h>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/md5.h>

#include <string>

#include "Base64.h"
#include "SMS4.h"

#define IS_AES(T)  strcmp(T, "AES") == 0 ? 1 : 0
#define IS_DES(T)  strcmp(T, "DES") == 0 ? 1 : 0
#define IS_RC4(T)  strcmp(T, "RC4") == 0 ? 1 : 0
#define IS_SM4(T)  strcmp(T, "SM4") == 0 ? 1 : 0

unsigned char iv[16] = {0x7d, 0xac, 0x31, 0x6, 0xaf, 0x9f, 0x69, 0xad, 0x3c, 0x47, 0xe9, 0x94, 0x9d, 0xd4, 0x6b, 0xd};    

void handleErrors(void)
{
	ERR_print_errors_fp(stderr);
	abort();
}

/*DES*/
extern "C" int Des_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
				unsigned char *iv, unsigned char *ciphertext)
{
	EVP_CIPHER_CTX *ctx;

	int len;
	int ciphertext_len;

	unsigned char ivTemp[16];
        memcpy(ivTemp, iv, 16);

	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	if(1 != EVP_EncryptInit_ex(ctx, EVP_des_cfb8(), NULL, key, ivTemp))
		handleErrors();

	if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
		handleErrors();
	ciphertext_len = len;

	if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
	ciphertext_len += len;

	EVP_CIPHER_CTX_free(ctx);

	EVP_cleanup();  
	ERR_free_strings();

	return ciphertext_len;
}

extern "C" int Des_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
				unsigned char *iv, unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx;

	int len;
	int plaintext_len;

	unsigned char ivTemp[16];
        memcpy(ivTemp, iv, 16);

	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	if(1 != EVP_DecryptInit_ex(ctx, EVP_des_cfb8(), NULL, key, ivTemp))
		handleErrors();

	if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		handleErrors();
	plaintext_len = len;

	if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
	plaintext_len += len;

	EVP_CIPHER_CTX_free(ctx);

	EVP_cleanup();  
	ERR_free_strings();

	return plaintext_len;
}

/*AES*/
extern "C" int Aes_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext)
{
  EVP_CIPHER_CTX *ctx;

  int len;
  int ciphertext_len;

  unsigned char ivTemp[16];
  memcpy(ivTemp, iv, 16);

  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cfb128(), NULL, key, ivTemp))
    handleErrors();

  if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    handleErrors();
  ciphertext_len = len;

  if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
  ciphertext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  EVP_cleanup();  
  ERR_free_strings();

  return ciphertext_len;
}

extern "C" int Aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext)
{
  EVP_CIPHER_CTX *ctx;

  int len;
  int plaintext_len;

  unsigned char ivTemp[16];
  memcpy(ivTemp, iv, 16);  

  if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cfb128(), NULL, key, ivTemp))
    handleErrors();

  if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    handleErrors();
  plaintext_len = len;

  if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  EVP_cleanup();  
  ERR_free_strings();

  return plaintext_len;
}

/*RC4*/
extern "C" int RC4_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
	EVP_CIPHER_CTX *ctx;

	int len;
	int ciphertext_len;

	unsigned char ivTemp[16];
        memcpy(ivTemp, iv, 16);

	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	if(1 != EVP_EncryptInit_ex(ctx, EVP_rc4(), NULL, key, ivTemp))
		handleErrors();

	if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
		handleErrors();
	ciphertext_len = len;

	if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
	ciphertext_len += len;

	EVP_CIPHER_CTX_free(ctx);

	EVP_cleanup();  
	ERR_free_strings();

	return ciphertext_len;
}

extern "C" int RC4_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx;

	int len;
	int plaintext_len;

	unsigned char ivTemp[16];
        memcpy(ivTemp, iv, 16);

	if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

	if(1 != EVP_DecryptInit_ex(ctx, EVP_rc4(), NULL, key, ivTemp))
		handleErrors();

	if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		handleErrors();
	plaintext_len = len;

	if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
	plaintext_len += len;

	EVP_CIPHER_CTX_free(ctx);

	EVP_cleanup();  
	ERR_free_strings();

	return plaintext_len;
}

/*sm4*/
extern "C" int SM4_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
	unsigned char ivTemp[16] = {0};
	memcpy(ivTemp, iv, 16);

	SMS4_CTX ctx;
	SMS4_SetKey(&ctx, key, ivTemp, 0);

	int uLen = SMS4_CFB_Encrypt(&ctx, plaintext, plaintext_len, ciphertext);
	
	return uLen;
}

extern "C" int SM4_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
	unsigned char ivTemp[16];
	memcpy(ivTemp, iv, 16);

	SMS4_CTX ctx;
	SMS4_SetKey(&ctx, key, ivTemp, 0);

	int uLen = SMS4_CFB_Decrypt(&ctx, ciphertext, ciphertext_len, plaintext);

	return uLen;
}

extern "C" int CEMS_encrypt(char* pType, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, unsigned char *key, int key_len, unsigned char *ivTemp)
{
	unsigned char szmd5sum[16];
	unsigned char ivUse[16];

	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, key, key_len);
	MD5_Final(szmd5sum, &ctx);

	if(ivTemp != NULL)
		memcpy(ivUse, ivTemp, 16);
	else
		memcpy(ivUse, iv, 16);

	int len = -1;

	if(IS_AES(pType))
        {
                len = Aes_encrypt(plaintext, plaintext_len, szmd5sum, ivUse, ciphertext); 
        }
        else if(IS_DES(pType))
        {
                len = Des_encrypt(plaintext, plaintext_len, szmd5sum, ivUse, ciphertext); 
        }
        else if(IS_RC4(pType))
        {
                len = RC4_encrypt(plaintext, plaintext_len, szmd5sum, ivUse, ciphertext); 
        }
        else if(IS_SM4(pType))
        {
                len = SM4_encrypt(plaintext, plaintext_len, szmd5sum, ivUse, ciphertext);
        }
	
	return len;	

}

extern "C" int CEMS_decrypt(char* pType, unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext, unsigned char *key, int key_len, unsigned char *ivTemp)
{
	unsigned char szmd5sum[16];
	unsigned char ivUse[16];

	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, key, key_len);
	MD5_Final(szmd5sum, &ctx);

	if(ivTemp != NULL)
                memcpy(ivUse, ivTemp, 16);
        else
                memcpy(ivUse, iv, 16);

	int len = -1;

	if(IS_AES(pType))
        {
                len = Aes_decrypt(ciphertext,  ciphertext_len, szmd5sum, ivUse, plaintext);
        }
        else if(IS_DES(pType))
        {
                len = Des_decrypt(ciphertext, ciphertext_len,  szmd5sum, ivUse, plaintext);
        }
        else if(IS_RC4(pType))
        {
                len = RC4_decrypt(ciphertext, ciphertext_len, szmd5sum, ivUse, plaintext);
        }
        else if(IS_SM4(pType))
        {
                len = SM4_decrypt(ciphertext, ciphertext_len, szmd5sum, ivUse, plaintext);
        }

	return len;
}

extern "C" std::string CEMS_Encrypt_Ex(std::string szType, std::string szPlainText, std::string szKey, unsigned char* iv)
{
	std::string szBase;

	int len = szPlainText.length();
	int keylen = szKey.size();

	unsigned char* pout = new unsigned char[len + 1];

	CEMS_encrypt((char*)szType.c_str(), (unsigned char*)szPlainText.c_str(), len, pout, (unsigned char*)szKey.data(), keylen, iv);

	CBase64 base;
	std::string szOut;
	base.Encode(pout, len, szOut);

	printf("type = %s, key = %s, base = %s\n", szType.c_str(), szKey.c_str(), szOut.c_str());

	szBase.assign((const char*)pout, len);

	delete []pout;
	return szBase;
}

extern "C" std::string CEMS_Decrypt_Ex(std::string szType, std::string szCipherText, std::string szKey, unsigned char* iv)
{
	std::string szOutPut;

	int len = szCipherText.size();
	int keylen = szKey.size();

	unsigned char* pout = new unsigned char[len + 1];
	bzero(pout, len + 1);

	CEMS_decrypt((char*)szType.c_str(), (unsigned char*)szCipherText.data(), len, pout, (unsigned char*)szKey.data(), keylen, iv);

	szOutPut.assign((const char*)pout, len + 1); 

	delete []pout;

	return szOutPut;
}
