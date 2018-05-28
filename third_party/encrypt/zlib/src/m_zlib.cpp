#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Base64.h"
#include "zlib.h"

#include "m_zlib.h"

#define  HEADER_LEN     50

/* Compress gzip data, data 原数据 ndata 原数据长度 zdata 压缩后数据 nzdata 压缩后长度 */
int gzcompress(Bytef *data, uLong ndata, Bytef *zdata, uLong *nzdata)
{
	z_stream c_stream;
	int err = 0;

	if(data && ndata > 0) 
	{
		c_stream.zalloc = NULL;
		c_stream.zfree = NULL;
		c_stream.opaque = NULL;
		
		/*只有设置为MAX_WBITS + 16才能在在压缩文本中带header和trailer*/
		if((err = deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY)) != Z_OK)
		{
			return err;
		}

		c_stream.next_in  = data;
		c_stream.avail_in  = ndata;
		c_stream.next_out = zdata;
		c_stream.avail_out  = *nzdata;

		while(c_stream.avail_in != 0 && c_stream.total_out < *nzdata) 
		{
			if((err = deflate(&c_stream, Z_NO_FLUSH)) != Z_OK)
			{
				return err;
			}
		}

		if(c_stream.avail_in != 0)
		{
			return c_stream.avail_in;
		}

		for(;;) 
		{
			if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
			if(err != Z_OK) return err;
		}

		if((err = deflateEnd(&c_stream)) != Z_OK)
		{
			return err;
		}

		*nzdata = c_stream.total_out;
		return Z_OK;
	}
	return Z_ERRNO;
}

/* Uncompress gzip data, zdata 数据 nzdata 原数据长度 data 解压后数据 ndata 解压后长度 */
int gzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata)
{
	int err = 0;
	z_stream d_stream = {0}; /* decompression stream */
	static char dummy_head[2] = {0x8 + 0x7 * 0x10, (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF};
	
	d_stream.zalloc = NULL;
	d_stream.zfree = NULL;
	d_stream.opaque = NULL;
	d_stream.next_in  = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	
	if(inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK) return Z_ERRNO;  /*只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本*/
	
	while(d_stream.total_out < *ndata && d_stream.total_in < nzdata) /*if(inflateInit2(&d_stream, 47) != Z_OK) return -1;*/
	{
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */

		if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
		{
			break;
		}

		if(err != Z_OK) 
		{
			if(err == Z_DATA_ERROR) 
			{
				d_stream.next_in = (Bytef*) dummy_head;
				d_stream.avail_in = sizeof(dummy_head);
				if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) 
				{
					return err;
				}
			} 
			else
			{
				return err;
			}
		}
	}

	if((err = inflateEnd(&d_stream)) != Z_OK)
	{
		return err;
	}

	*ndata = d_stream.total_out;
	return Z_OK;
}

extern "C" int deflate_compress(unsigned char *bufin, int lenin, unsigned char *bufout, int lenout)
{
	uLong uSlen = lenout;
	uLong usize;
	usize  = compressBound(uSlen);
	int err = compress((Bytef*)bufout, &uSlen, (Bytef*)bufin, lenin);

	return err == Z_OK ? uSlen : err;
}

extern "C" int deflate_uncompress(unsigned char *bufin, int lenin, unsigned char *bufout, int lenout)
{
	uLong uSlen = lenout;
	int err = uncompress((Bytef*)bufout, &uSlen, (Bytef*)bufin, lenin);

	return err == Z_OK ? uSlen : err;
}


extern "C" int gzip_compress(unsigned char *bufin, int lenin, unsigned char *bufout, int lenout)
{
	uLong uSlen = lenout;
	int ret = gzcompress((Bytef*)bufin, lenin, (Bytef*)bufout, &uSlen);

	return ret == Z_OK ? uSlen : ret;
}

extern "C" int gzip_uncompress(unsigned char *bufin, int lenin, unsigned char *bufout, int lenout)
{
	uLong uSlen = lenout;
	int ret = gzdecompress((Byte*)bufin, lenin, (Byte*)bufout, &uSlen);

	return ret == Z_OK ? uSlen : ret;
}

/*int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("参数格式错误\n");

		printf("useage:\n");
		printf("zlib  /deflate\t\"汉字123abc~!@#$%^&*(\"\n");
		printf("zlib  /gzip \t\"汉字123abc~!@#$%^&*(\"\n");
		return -1;
	}

	static int i = 1;
begin:
	i++;
	printf("压缩内容: %s\n", argv[2]);

	int result;
	std::string szSrc = argv[2];

	char *bufin;
	bufin = (char*)szSrc.c_str();

	 char *bufout = NULL;
	 char *bufout2 = NULL;

	 uLong lenout;
	 int   lensrc = strlen(bufin);

	 char *pType = argv[1];

	 printf("压缩方式: %s\n", pType);
	 printf("压缩前：%d 字节\n", strlen(bufin));

	 int CompressedLen = lensrc + HEADER_LEN;

	 bufout = new char[CompressedLen + 1];
	 bzero(bufout, CompressedLen + 1);

	
	 if(strcmp(pType, "/deflate") == 0)
	 {
		lenout = deflate_compress((unsigned char*)bufin, lensrc, (unsigned char*)bufout, CompressedLen);
	 }
	 else if(strcmp(pType, "/gzip") == 0)
	 {
		lenout = gzip_compress((unsigned char*)bufin, lensrc, (unsigned char*)bufout, CompressedLen);
	 }

	 std::string szBaseGzip;
	 CBase64::Encode((unsigned char*)bufout, (long)lenout, szBaseGzip);

	 printf("压缩后: base64 ： %s\n",  szBaseGzip.c_str());

	 printf("压缩后：%d 字节\n",  lenout);

#ifndef MY_DEBUG
	 for(int i = 0; i < lenout; i++)
	 {
		 printf("0x%02x ", (unsigned char)bufout[i]);
		 if((i + 1) % 8 == 0)
		 {
			printf("\n");
		 }
	 }
	 printf("\n");
#endif

	 bzero(bufout, lenout);
	 CBase64::Decode(szBaseGzip, (unsigned char*)bufout, &lenout);

	 bufout2 = new char[lensrc + 1];
	 bzero(bufout2, lensrc + 1);

	 if(strcmp(pType, "/deflate") == 0)
	 {
		 deflate_uncompress((unsigned char*)bufout, lenout, (unsigned char*)bufout2, lensrc);
	 }
	 else if(strcmp(pType, "/gzip") == 0)
	 {
		 gzip_uncompress((unsigned char*)bufout, lenout, (unsigned char*)bufout2, lensrc);
	 }

	 printf("解压缩后: %s\n", bufout2);

	 delete []bufout;
	 delete []bufout2;

	 if(i > 2 )
		 goto end;

	 goto begin;

end:
	return 0;	
}*/

