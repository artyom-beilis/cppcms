///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/config.h>
#include <cppcms/session_cookies.h>
#include <cppcms/crypto.h>
#include "hmac_encryptor.h"
#if defined(CPPCMS_HAVE_GCRYPT) || defined(CPPCMS_HAVE_OPENSSL)
#include "aes_encryptor.h"
#endif
#include "test.h"
#include <iostream>
#include <string.h>
#include <time.h>
#include <stdio.h>

template<typename EncryptorFactory>
std::auto_ptr<cppcms::sessions::encryptor> gen(std::string const &name,cppcms::crypto::key const &k)
{
	std::auto_ptr<cppcms::sessions::encryptor_factory> fact(new EncryptorFactory(name,k));
	return fact->get();
}

template<typename Encryptor>
void run_test(std::string name,std::string k,bool is_signature = false) 
{
	using cppcms::crypto::key;
	char c1=k[0];
	char c2=k[31];
	std::auto_ptr<cppcms::sessions::encryptor> enc = gen<Encryptor>(name,key(k));
	std::string cipher=enc->encrypt("Hello World");
	std::string plain;
	TEST(enc->decrypt(cipher,plain));
	TEST(plain=="Hello World");

	std::string orig_text[2]= { "", "x" };
	for(unsigned t=0;t<2;t++) {
		cipher=enc->encrypt(orig_text[t]);
		TEST(enc->decrypt(cipher,plain));
		TEST(plain==orig_text[t]);

		for(unsigned i=0;i<cipher.size();i++) {
			cipher[i]--;
			TEST(!enc->decrypt(cipher,plain));
			cipher[i]+=2;
			TEST(!enc->decrypt(cipher,plain));
			cipher[i]--;
		}
	}
	TEST(!enc->decrypt("",plain));
	std::string a(0,64);
	TEST(!enc->decrypt(a,plain));
	std::string b(0xFF,64);
	TEST(!enc->decrypt(b,plain));
	cipher=enc->encrypt("");
	TEST(enc->decrypt(cipher,plain) && plain=="");
	cipher=enc->encrypt("test");
	k[0]=c1+1;
	enc = gen<Encryptor>(name,key(k));
	TEST(!enc->decrypt(cipher,plain));
	k[0]=c1;
	k[31]=c2+1;
	enc = gen<Encryptor>(name,key(k));
	TEST(!enc->decrypt(cipher,plain));
	k[31]=c2;
	enc = gen<Encryptor>(name,key(k));
	TEST(enc->decrypt(cipher,plain) && plain=="test");
	// Salt works
	if(!is_signature) {
		TEST(enc->encrypt(plain)!=enc->encrypt(plain));
		enc = gen<Encryptor>(name,key(k));
		std::string a=enc->encrypt(plain);
		enc = gen<Encryptor>(name,key(k));
		std::string b=enc->encrypt(plain);
		TEST(a!=b);
		plain="XX";
		TEST(enc->decrypt(a,plain) && plain == "test");
	}
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
	using namespace cppcms::crypto;
	std::cout << "-- testing sha1/md5" << std::endl;
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
	std::cout << "-- testing hmac-sha1/md5" << std::endl;
	{
		hmac d("md5",key("Jefe",4));
		TEST(get_diget(d,"what do ya want for nothing?") == "750c783e6ab0b503eaa86e310a5db738");
	}
	{
		char const *bk = "xxxxxxxxxxxxxxddddddddddddddddddffffffffffffffffffffffffffffffffffffffffffffffdddddddddddddddddddd";
		hmac d(message_digest::md5(),key(bk,strlen(bk)));
		TEST(get_diget(d,"what do ya want for nothing?") == "4891f8cf6a4641897159756847369d1a");
	}

	#if defined(CPPCMS_HAVE_GCRYPT) || defined(CPPCMS_HAVE_OPENSSL)
	std::cout << "-- testing shaXXX " << std::endl;
	{
		std::auto_ptr<message_digest> d(message_digest::create_by_name("sha256"));
		TEST(d->name() == std::string("sha256"));
		TEST(d->block_size() == 64);
		TEST(get_diget(*d,"Hello World")=="a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e");
		d = message_digest::create_by_name("sha512");
		TEST(d->name() == std::string("sha512"));
		TEST(d->digest_size() == 64);
		TEST(d->block_size() == 128);
	}
	#endif

}

void test_key(std::string test_dir)
{
	std::cout << "- Testing key" << std::endl;
	cppcms::crypto::key k;
	k.set_hex("aabb",4);
	TEST(k.size()==2);
	TEST(memcmp(k.data(),"\xaa\xbb",2)==0);
	k.set("aabb",4);
	TEST(k.size()==4);
	TEST(memcmp(k.data(),"aabb",4)==0);
	k=cppcms::crypto::key(std::string("ab"));
	TEST(k.size()==1);
	TEST(*k.data()=='\xAB');
	cppcms::crypto::key k2(k);
	TEST(k2.size()==1);
	TEST(*k2.data()=='\xAB');
	cppcms::crypto::key k3;
	TEST(k3.size()==0);
	k3=k;
	TEST(k3.size()==1);
	TEST(*k3.data()=='\xAB');
	k.read_from_file(test_dir+"/sample_key.txt");
	TEST(k.size()==4);
	TEST(memcmp(k.data(),"\xab\x01\xcc\xdd",4)==0);
}

int main(int argc,char **argv)
{
	if(argc!=2) {
		std::cerr << "usage: provide test directory " << std::endl;
		return 1;
	}
	try {
		test_key(argv[1]);
		std::cout << "- Testing basic cryptography functions" << std::endl;

		std::string key="261965ba80a79c034c9ae366a19a2627";

		std::cout << "- Testing internal cryptography library" << std::endl;
		std::cout << "-- Testing hmac cookies signature" << std::endl;
		run_test<cppcms::sessions::impl::hmac_factory>("sha1",key,true);
		run_test<cppcms::sessions::impl::hmac_factory>("md5",key,true);
		#if defined(CPPCMS_HAVE_GCRYPT) || defined(CPPCMS_HAVE_OPENSSL)
		std::cout << "- Testing external cryptography library" << std::endl;
		std::cout << "-- Testing hmac cookies signature" << std::endl;
		std::cout << "--- hmac-sha224" << std::endl;
		run_test<cppcms::sessions::impl::hmac_factory>("sha224",key,true);
		std::cout << "--- hmac-sha256" << std::endl;
		run_test<cppcms::sessions::impl::hmac_factory>("sha256",key,true);
		std::cout << "--- hmac-sha384" << std::endl;
		run_test<cppcms::sessions::impl::hmac_factory>("sha384",key,true);
		std::cout << "--- hmac-sha512" << std::endl;
		run_test<cppcms::sessions::impl::hmac_factory>("sha512",key,true);
		std::cout << "-- Testing aes cookies encryption" << std::endl;
		std::cout << "--- aes" << std::endl;
		run_test<cppcms::sessions::impl::aes_factory>("aes",key);
		std::cout << "--- aes128" << std::endl;
		run_test<cppcms::sessions::impl::aes_factory>("aes128",key);
		std::cout << "--- aes192" << std::endl;
		run_test<cppcms::sessions::impl::aes_factory>("aes192",key + key.substr(16));
		std::cout << "--- aes256" << std::endl;
		run_test<cppcms::sessions::impl::aes_factory>("aes256",key + key);
		#endif
	}
	catch(std::exception const &e) {
		std::cerr << "Failed:"<<e.what()<<std::endl;
		return 1;
	}
	return 0;
}



