#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>

typedef unsigned char BYTE;
std::string base64_encode(const BYTE *buf, unsigned int bufLen);
std::vector<BYTE> base64_decode(std::string const&);

#endif
