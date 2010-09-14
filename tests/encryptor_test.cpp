///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/config.h>
#include <cppcms/session_cookies.h>
#include <cppcms/crypto.h>
#include "hmac_encryptor.h"
#ifdef CPPCMS_HAVE_GCRYPT
#include "aes_encryptor.h"
#endif
#include "test.h"
#include <iostream>
#include <string.h>
#include <time.h>


template<typename Encryptor>
void run_test() 
{
	std::string key="261965ba80a79c034c9ae366a19a2627";
	char c1=key[0];
	char c2=key[31];
	std::auto_ptr<cppcms::sessions::encryptor> enc(new Encryptor(key));
	time_t now=time(0)+5;
	std::string cipher=enc->encrypt("Hello World",now);
	std::string plain;
	time_t time;
	TEST(enc->decrypt(cipher,plain,&time));
	TEST(plain=="Hello World");
	TEST(time==now);

	std::string orig_text[2]= { "", "x" };
	for(unsigned t=0;t<2;t++) {
		cipher=enc->encrypt(orig_text[t],now+3);
		TEST(enc->decrypt(cipher,plain,&time));
		TEST(plain==orig_text[t]);
		TEST(time==now+3);

		for(unsigned i=0;i<cipher.size();i++) {
			cipher[i]--;
			TEST(!enc->decrypt(cipher,plain,&time) || plain==orig_text[t]);
			cipher[i]+=2;
			TEST(!enc->decrypt(cipher,plain,&time) || plain==orig_text[t]);
			cipher[i]--;
		}
	}
	TEST(!enc->decrypt("",plain,&time));
	std::string a(0,64);
	TEST(!enc->decrypt(a,plain,&time));
	std::string b(0xFF,64);
	TEST(!enc->decrypt(b,plain,&time));
	cipher=enc->encrypt("",now-6);
	TEST(!enc->decrypt(cipher,plain,&time));
	cipher=enc->encrypt("test",now);
	key[0]=c1+1;
	enc.reset(new Encryptor(key));
	TEST(!enc->decrypt(cipher,plain,&time));
	key[0]=c1;
	key[31]=c2+1;
	enc.reset(new Encryptor(key));
	TEST(!enc->decrypt(cipher,plain,&time));
	key[31]=c2;
	enc.reset(new Encryptor(key));
	TEST(enc->decrypt(cipher,plain,&time) && plain=="test" && time==now);
	// Salt works
	TEST(enc->encrypt(plain,now)!=enc->encrypt(plain,now));
}


std::string to_string(unsigned char *ptr,size_t n)
{
	std::string res;
	for(unsigned i=0;i<n;i++) {
		char buf[3];
		sprintf(buf,"%02x",ptr[i]);
		res+=buf;
	}
	return res;
}

template<typename MD>
std::string get_diget(MD &d,std::string const &source)
{
	unsigned char buf[256];
	d.append(source.c_str(),source.size());
	unsigned n=d.digest_size();
	d.readout(buf);
	return to_string(buf,n);
}

void test_crypto()
{
	using namespace cppcms;
	TEST(message_digest::md5().get());
	TEST(message_digest::sha1().get());
	TEST(message_digest::md5()->name() == std::string("md5"));
	TEST(message_digest::sha1()->name() == std::string("sha1"));
	TEST(message_digest::md5()->block_size() == 64);
	TEST(message_digest::sha1()->block_size() == 64);
	TEST(message_digest::md5()->digest_size() == 16);
	TEST(message_digest::sha1()->digest_size() == 20);
	{
		std::auto_ptr<message_digest> d(message_digest::create_by_name("md5"));
		TEST(d->name() == std::string("md5"));
		TEST(get_diget(*d,"")=="d41d8cd98f00b204e9800998ecf8427e");
		TEST(get_diget(*d,"Hello World!")=="ed076287532e86365e841e92bfc50d8c");
	}
	{
		std::auto_ptr<message_digest> d(message_digest::create_by_name("sha1"));
		TEST(d->name() == std::string("sha1"));
		TEST(get_diget(*d,"")=="da39a3ee5e6b4b0d3255bfef95601890afd80709");
		TEST(get_diget(*d,"Hello World!")=="2ef7bde608ce5404e97d5f042f95f89f1c232871");
	}
	{
		hmac d("md5","Jefe");
		TEST(get_diget(d,"what do ya want for nothing?") == "750c783e6ab0b503eaa86e310a5db738");
	}
	{
		hmac d(message_digest::md5(),"xxxxxxxxxxxxxxddddddddddddddddddffffffffffffffffffffffffffffffffffffffffffffffdddddddddddddddddddd");
		TEST(get_diget(d,"what do ya want for nothing?") == "4891f8cf6a4641897159756847369d1a");
	}

}

int main()
{
	try {
		std::cout << "Testing basic cryptography" << std::endl;

		test_crypto();

		std::cout << "Testing hmac" << std::endl;
		run_test<cppcms::sessions::impl::hmac_cipher>();
		#ifdef CPPCMS_HAVE_GCRYPT
		std::cout << "Testing aes" << std::endl;
		run_test<cppcms::sessions::impl::aes_cipher>();
		#endif
	}
	catch(std::exception const &e) {
		std::cerr << "Failed:"<<e.what()<<std::endl;
		return 1;
	}
	return 0;
}



