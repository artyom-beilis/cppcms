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
	bool ok = memcmp(&mac[0],cipher.c_str() + message_size,digest_size) == 0;
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
