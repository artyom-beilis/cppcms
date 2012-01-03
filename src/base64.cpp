/*
 *  Copyright: (c) 2008-2012 Artyom Beilis
 *  License: MIT
 *  Other Copyrights: 
 *    7 lines of code had taken from: http://base64.sourceforge.net/b64.c
 *    and they are subject to MIT license by (c) Trantor Standard Systems Inc., 2001
 */

#define CPPCMS_SOURCE
#include <cppcms/base64.h>
#include <vector>
#include <ostream>


namespace {

const unsigned char encode_6_to_8[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

inline unsigned char encode_8_to_6(unsigned char c)
{
	if('A'<=c && c<='Z')
		return   c - 'A';
	if('a'<=c && c<='z')
		return   ('Z'-'A' + 1) + c - 'a';
	if('0'<=c && c<='9')
		return 2*('Z'-'A' + 1) + c - '0';
	if(c=='-')
		return 62;
	if(c=='_')
		return 63;
	return 0;
}

size_t inline bencode(unsigned const char in[3],unsigned char out[4],size_t len)
{
	out[0] = encode_6_to_8[in[0] >> 2];
	if(len<=1) {
		out[1] = encode_6_to_8[(in[0] & 0x03) << 4];
		return 2;
	}
	else {
		out[1] = encode_6_to_8[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
		if(len<=2) {
			out[2] = encode_6_to_8[(in[1] & 0x0f) << 2];
			return 3;
		}
		else {
			out[2] = encode_6_to_8[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)];
			out[3] = encode_6_to_8[in[2] & 0x3f];
			return 4;
		}
	}
}

inline size_t bdecode(unsigned const char in8[4],unsigned char out[3],size_t len)
{
	unsigned char in[4] = { 0 };
	for(unsigned i=0;i<len;i++)
		in[i]=encode_8_to_6(in8[i]);
	
	out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
	if(len==2) {
		return 1;
	} 
	else {
		out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
		if(len==3) {
			return 2;
		}
		else {
			out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
			return 3;
		}
	}
}

} // anon namespace

namespace cppcms {

namespace b64url {

int encoded_size(size_t s)
{
	switch(s % 3) {
	case 1: return s/3*4+2;
	case 2: return s/3*4+3;
	default:
		return s/3*4;
	}
}

int decoded_size(size_t s)
{
	switch(s % 4) {
	case 1: return -1; // invalid
	case 2: return s/4*3+1;
	case 3: return s/4*3+2;
	default:
		return s/4*3;
	}
}

unsigned char *encode(unsigned char const *begin,unsigned char const *end,unsigned char *target)
{
	while(end - begin >=3) {
		bencode(begin,target,3);
		begin += 3;
		target += 4;
	}
	if(end!=begin)
		target+=bencode(begin,target,end-begin);
	return target;
}

void encode(unsigned char const *begin,unsigned char const *end,std::ostream &out)
{
	unsigned char target[4];
	while(end - begin >=3) {
		bencode(begin,target,3);
		begin += 3;
		out.write(reinterpret_cast<char *>(target),4);
	}
	if(end!=begin) {
		int n = bencode(begin,target,end-begin);
		out.write(reinterpret_cast<char *>(target),n);
	}
}


unsigned char *decode(unsigned char const *begin,unsigned char const *end,unsigned char *target)
{
	while(end - begin >=4) {
		bdecode(begin,target,4);
		begin+=4;
		target+=3;
	}
	if(end!=begin)
		target+=bdecode(begin,target,end-begin);
	return target;
}

bool CPPCMS_API decode(std::string const &input,std::string &output)
{
	int ds = decoded_size(input.size());
	if(ds < 0)
		return false;
	if(ds == 0)
		return true;
	unsigned char const *begin = reinterpret_cast<unsigned char const *>(input.c_str());
	unsigned char const *end = begin + input.size();
	std::vector<char> buf(ds,0);
	unsigned char *outbuf =  reinterpret_cast<unsigned char *>(&buf[0]);
	decode(begin,end,outbuf);
	output.assign(&buf[0],ds);
	return true;
}
///
/// Perform base64 URL encoding of the binary data \a input, and return it
///
///
std::string CPPCMS_API encode(std::string const &input)
{
	std::string result;
	size_t enc_size = encoded_size(input.size());
	if(enc_size == 0)
		return result;
	unsigned char const *begin = reinterpret_cast<unsigned char const *>(input.c_str());
	unsigned char const *end = begin + input.size();
	std::vector<char> buf(enc_size,0);
	unsigned char *outbuf =  reinterpret_cast<unsigned char *>(&buf[0]);
	encode(begin,end,outbuf);
	result.assign(&buf[0],enc_size);
	return result;
}


} // b64url
} // cppcms 


