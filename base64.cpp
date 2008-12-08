/*
 *  Copyright: (c) 2008 Artyom Beilise
 *  License: MIT
 *  Other Copyrights: 
 *    7 lines of code had taken from: http://base64.sourceforge.net/b64.c
 *    and they are subject to MIT license by (c) Trantor Standard Systems Inc., 2001
 */

#include "base64.h"

#ifdef UNIT_TEST

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <string>

#endif

 


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

ssize_t encoded_size(size_t s)
{
	switch(s % 3) {
	case 1: return s/3*4+2;
	case 2: return s/3*4+3;
	default:
		return s/3*4;
	}
}

ssize_t decoded_size(size_t s)
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


} // b64url
} // cppcms 

#ifdef UNIT_TEST

using namespace std;
using namespace cppcms;

int test1()
{
	static unsigned char out[5];
	unsigned char msg[]="sur";
	bencode(msg,out,3);
	if(!equal(out,out+4,"c3Vy")) {
		cerr<<"Faile "<<out<<endl;
	}
	for(unsigned v=0;v<0xFFFFFF;v++) {
		unsigned char in[3],in2[3];
		unsigned char out[4];
		in[0]=v & 0xFF;
		in[1]=(v>>8) & 0xFF;
		in[2]=(v>>16);
		for(int i=1;i<=3;i++){
			memset(in2,0,3);
			int n=bencode(in,out,i);
			bdecode(out,in2,n);
			if(!equal(in,in+i,in2)) {
				printf("%06X %d %d\n",v,i,n);
				return 1;
			}
		}
	}
	return 0;
}

int test2()
{
	unsigned i;
	for(i=0;i<10000;i++) {
		unsigned len=rand() % 1000;
		vector<unsigned char> in(len,0);
		for(unsigned j=0;j<len;j++) {
			in[j]=rand();
		}

		vector<unsigned char> tmp(b64url::encoded_size(len),0);
		b64url::encode(&in.front(),&in.front()+len,&tmp.front());
		vector<unsigned char> out(b64url::decoded_size(tmp.size()),0);
		b64url::decode(&tmp.front(),&tmp.front()+tmp.size(),&out.front());

		if(in.size()!=out.size()) {
			cerr<<"Size: "<<in.size()<<" "<<out.size()<<endl;
			return 1;
		}
		if(!equal(in.begin(),in.end(),out.begin())) {
			string str(tmp.begin(),tmp.end());
			for(unsigned j=0;j<in.size();j++) {
				cerr<<int(in[j])<<" "<<int(out[j])<<endl;
			}
			cerr<<str<<endl;
			cerr<<"Failed "<<len<<endl;
			return 1;
		}
	}
	cout<<"Ok "<<i<<"\n" ;
	return 0;
}

int main()
{
	return test1()==0 && test2()==0 ? 0 : 1;
}

#endif
