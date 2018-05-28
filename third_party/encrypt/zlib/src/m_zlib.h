#ifndef _M_ZLIB_H
#define _M_ZLIB_H

#include <string>

/*len 返回压缩后的长度*/
std::string MCompression(std::string szSrc, unsigned long & len);

/*slen 压缩前的数据长度*/
std::string MUncompression(std::string szCompressed, unsigned long & slen);

#endif
