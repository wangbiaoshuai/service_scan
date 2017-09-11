#ifndef _MJD_GENERAL_
#define _MJD_GENERAL_

enum  CRYPT_TYPE
{
   NEC = 0,
   DES = 1,
   AES,
   RC4,
   SM4,
};

enum ZLIB_TYPE
{
     DEFAULT = 0,
     DEFLATE = 1,
     GZIP,
};

/*crypto*/
typedef int (*pCEMS_encrypt)(char*, unsigned char *, int , unsigned char *, unsigned char *, int, unsigned char *);
typedef int (*pCEMS_decrypt)(char*, unsigned char *, int , unsigned char *, unsigned char *, int, unsigned char *);

typedef int (*pSM4_encrypt)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *);
typedef int (*pSM4_decrypt)(unsigned char *, int, unsigned char *, unsigned char *, unsigned char *);

typedef std::string (*pCEMS_Encrypt_Ex)(std::string, std::string, std::string, unsigned char*);
typedef std::string (*pCEMS_Decrypt_Ex)(std::string, std::string, std::string, unsigned char*);

/*zlib*/
typedef int (* pdeflate_compress)(unsigned char *, int, unsigned char *, int);
typedef int (* pdeflate_uncompress)(unsigned char *, int, unsigned char *, int);

typedef int (* pgzip_compress)(unsigned char *, int, unsigned char *, int);
typedef int (* pgzip_uncompress)(unsigned char *, int, unsigned char *, int);

class CGenAlgori
{
public:
	CGenAlgori();
	~CGenAlgori();

private:
	void * m_zlibHandle;
	void * m_cryptHandle;

private:
	pdeflate_compress	m_pdeflate_compress;
	pdeflate_uncompress	m_pdeflate_uncompress;

	pgzip_compress		m_pgzip_compress;
	pgzip_uncompress	m_pgzip_uncompress;

private:
	pCEMS_encrypt		m_pCEMS_encrypt;
	pCEMS_decrypt		m_pCEMS_decrypt;

	pSM4_encrypt		m_pSM4_encrypt;
	pSM4_decrypt		m_pSM4_decrypt;

	pCEMS_Encrypt_Ex	m_pCEMS_Encrypt_Ex;
	pCEMS_Decrypt_Ex	m_pCEMS_Decrypt_Ex;

public:
	void load();
	void unload();

public:
	std::string EnCrypt(std::string, unsigned int type, std::string password);
	std::string DeCrypt(std::string, unsigned int type, std::string password);

	std::string Compress(std::string, unsigned int type);
	std::string UnCompress(std::string, unsigned int type);	
	
};

#endif
