///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/encoding.h>
#include <iostream>
#include <iconv.h>
#include <stdlib.h>

using namespace std;
using namespace cppcms;
using namespace cppcms::encoding;
validators_set valid;


int is_legal_special(int c)
{
	if(c=='\t' || c=='\r' || c=='\n')
		return true;
	if(0x20<=c && c<=0x7E)
		return true;
	if(c>=0xA0)
		return true;
	return false;
}

void test8bit(string enc,unsigned range)
{
	iconv_t d=iconv_open(native_unicode_encoding(),enc.c_str());
	if(d==(iconv_t)(-1)) {
		cerr<<"Warning failed to test encoding "<<enc<<", it is not supported by iconv"<<endl;
		return;
	}
	for(unsigned i=0;i<=range;i++) {
		char cbuf[1];
		cbuf[0]=i;
		char *p_cbuf=cbuf;
		union { uint32_t ubuf; char ubufc[4]; } buf;
		char *p_ubuf=buf.ubufc;
		size_t inleft=1;
		size_t outleft=4;
		iconv(d,0,0,0,0);
		buf.ubuf=0;
		size_t res=iconv(d,&p_cbuf,&inleft,&p_ubuf,&outleft);
		bool ok=true;
		if(res==(size_t)(-1)) {
			ok=false;
		}
		else if(outleft!=0) {
			ok = iconv(d,0,0,&p_ubuf,&outleft) !=(size_t)(-1);
		}
		if(enc=="latin1" && (!ok || buf.ubuf!=i)) {
			cerr<<"Internal tester error"<<endl;
			exit(1);
		}
		ok= ok && is_legal_special(buf.ubuf);
		bool test=valid[enc].valid(cbuf,cbuf+1);
		if(ok!=test) {
			cerr<<"Test of "<<enc<<" Failed at "<<hex<<i<<" Charrecter , as 0x"<<buf.ubuf<<endl;
			cerr<<"Iconv:\t\t"<<(ok ? "accepted" : "rejected")<<endl;
			cerr<<"Validator:\t"<<(test ? "accepted" : "rejected")<<endl;
			exit(1);
		}

	}
	cout<<enc<<" Ok"<<endl;
	iconv_close(d);
}

void unitest(std::string enc)
{
	char c=0;

	iconv_t d=iconv_open(native_unicode_encoding(),enc.c_str());

	validator val=valid[enc];
	if(val.valid(&c,&c+1)) { cerr<<"Failed on charrecter 0"<< endl; exit(1); }
	unsigned min_len=0;
	for(unsigned i=1;i!=0;i++) {
		uint32_t range=i;
		while((0xFF000000u & range)==0) {
			range<<=8;
		}
		size_t in_size=0,orig_size;
		char buf[4];
		do {
			buf[in_size++]=range>>24;
			range<<=8;
		}while(0xFF000000u & range);
		orig_size=in_size;
		if(orig_size < min_len)
			continue;
		min_len=orig_size;
		

		union { uint32_t u[4]; char r[16]; } u;
		char *input=buf;
		char *output=u.r;
		size_t out_size=16;

		iconv(d,0,0,0,0);
		bool ok = iconv(d,&input,&in_size,&output,&out_size)!=(size_t)(-1);
		ok = ok && iconv(d,0,0,&output,&out_size) !=(size_t)(-1);
		unsigned size=(16-out_size)/4;


		for(unsigned j=0;ok && j<size;j++)
			ok = ok && is_legal_special(u.u[j]);

		bool test=val.valid(buf,buf+orig_size);
		
		if(i % (0xFFFFFFFFu/100u) == 0) {
			cout<<(i / (0xFFFFFFFFu/100u))<<"% passed"<<endl;
		}

		if(ok!=test) {
			cerr<<"Test of "<<enc<<" Failed at "<<hex<<range<<" Charrecter"<<endl;
			cerr<<"Iconv:\t\t"<<(ok ? "accepted" : "rejected")<<endl;
			cerr<<"Validator:\t"<<(test ? "accepted" : "rejected")<<endl;
			exit(1);
		}
	}
	iconv_close(d);
}

int cppcms_validator_test_function()
{
	map<string,encoding_tester_type>::const_iterator p;
	for(p=valid.predefined_.begin();p!=valid.predefined_.end();++p) {
		string encoding=p->first;
		cout<<"Testing "<<encoding<<endl;
		if(encoding.substr(0,3)=="utf")
			unitest(encoding);
		if(encoding=="us-ascii" || encoding=="ascii")
			test8bit(encoding,127);
		else
			test8bit(encoding,255);
	}
	return 0;
}


int main()
{
	cppcms_validator_test_function();
	return 0;
}
