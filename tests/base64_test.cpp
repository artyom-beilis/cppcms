#include "base64.h"

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <string>


using namespace std;
using namespace cppcms;
using namespace cppcms::b64url;

int test1()
{
	static unsigned char out[5];
	unsigned char msg[]="sur";
	encode(msg,msg+3,out);
	if(!equal(out,out+4,"c3Vy")) {
		cerr<<"Faile "<<out<<endl;
		return 1;
	}
	for(unsigned v=0;v<0xFFFFFF;v++) {
		unsigned char in[3],in2[3];
		unsigned char out[4];
		in[0]=v & 0xFF;
		in[1]=(v>>8) & 0xFF;
		in[2]=(v>>16);
		for(int i=1;i<=3;i++){
			memset(in2,0,3);
			int n=encode(in,in+i,out)-out;
			decode(out,out+n,in2);
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

