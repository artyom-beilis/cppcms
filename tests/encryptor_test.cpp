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
#include "hmac_encryptor.h"
#ifdef HAVE_GCRYPT
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


int main()
{
	try {

		std::cout << "Testing hmac" << std::endl;
		run_test<cppcms::sessions::impl::hmac_cipher>();
		#ifdef HAVE_GCRYPT
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



