///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "hmac_encryptor.h"
#include "md5.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <cppcms/crypto.h>
#include <cppcms/cppcms_error.h>

namespace cppcms {
namespace sessions {
namespace impl {

hmac_factory::hmac_factory(std::string const &algo,crypto::key const &k) :
	algo_(algo),
	key_(k)
{
}

std::auto_ptr<encryptor> hmac_factory::get()
{
	std::auto_ptr<encryptor> ptr;
	ptr.reset(new hmac_cipher(algo_,key_));
	return ptr;
}


hmac_cipher::hmac_cipher(std::string const &hash_name,crypto::key const &k) :
	key_(k),
	hash_(hash_name)
{
	if(key_.size() < 16) {
		throw cppcms_error("The key legth is too small, use at leaset the key of 16 bytes/32 hexadecimal digits");
	}
}

std::string hmac_cipher::encrypt(std::string const &plain)
{
	crypto::hmac md(hash_,key_);
	
	size_t message_size = plain.size();
	size_t digest_size = md.digest_size();
	size_t cipher_size = message_size + digest_size;
	std::vector<char> data(cipher_size,0);
	md.append(plain.c_str(),plain.size());
	memcpy(&data[0],plain.c_str(),plain.size());
	md.readout(&data[message_size]);
	
	return std::string(&data[0],cipher_size);
}

bool hmac_cipher::equal(void const *a,void const *b,size_t n)
{
	char const *left = static_cast<char const *>(a);
	char const *right = static_cast<char const *>(b);
	size_t diff = 0;
	for(size_t i=0;i<n;i++) {
		if(left[i]!=right[i])
			diff++;
	}
	return diff==0;
}

bool hmac_cipher::decrypt(std::string const &cipher,std::string &plain)
{
	crypto::hmac md(hash_,key_);
	
	size_t cipher_size = cipher.size();
	size_t digest_size = md.digest_size();
	if(cipher_size < digest_size)
		return false;
	size_t message_size = cipher_size - digest_size;
	md.append(cipher.c_str(),message_size);
	std::vector<char> mac(digest_size,0);
	md.readout(&mac[0]);
	bool ok = equal(&mac[0],cipher.c_str() + message_size,digest_size);
	memset(&mac[0],0,digest_size);
	if(ok) {
		plain = cipher.substr(0,message_size);
		return true;
	}
	return false;
}

} // impl
} // sessions
} // cppcms
