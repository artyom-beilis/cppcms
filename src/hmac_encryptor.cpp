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

/*******************************************
 * The layout of the message is following:
 *  Signed Content
 *             8 bytes timeout;
 *             4 bytes message_size;
 *  message_size bytes content
 *  Signature
 *             N bytes - signature
 *******************************************/

hmac_cipher::hmac_cipher(std::string key,std::string hash_name) :
	key_(to_binary(key)),
	hash_(hash_name)
{
	if(key_.size() < 16) {
		throw cppcms_error("The key legth is too small, use at leaset the key of 16 bytes/32 hexadecimal digits");
	}
}

std::string hmac_cipher::encrypt(std::string const &plain,time_t timeout)
{
	hmac md(hash_,key_);
	
	int64_t real_timeout = timeout;
	uint32_t message_size = plain.size();
	const unsigned digest_size = md.digest_size();
	unsigned const content_size = sizeof(real_timeout) + sizeof(message_size) + message_size;
	std::vector<unsigned char> data(content_size + digest_size);
	unsigned char *signed_content = &data[0];
	unsigned char *time_content   = &data[0];
	unsigned char *size_content   = &data[0+8];
	unsigned char *plain_content  = &data[0+8+4];
	unsigned char *digest         = &data[0+8+4+message_size];
	memcpy(time_content,&real_timeout,8);
	memcpy(size_content,&message_size,4);
	memcpy(plain_content,plain.c_str(),message_size);
	md.append(signed_content,content_size);
	md.readout(digest);
	
	return base64_enc(data);
}

bool hmac_cipher::decrypt(std::string const &cipher,std::string &plain,time_t *timeout)
{
	std::vector<unsigned char> data;
	base64_dec(cipher,data);

	hmac md(hash_,key_);

	//
	// validation
	//
	
	// basic size
	const int64_t digest_size = md.digest_size();
	int64_t calculated_content_size = int64_t(data.size()) - int64_t(digest_size) - 8 - 4;
	if(calculated_content_size < 0)
		return false;
	// time and size 
	unsigned char *signed_body  = &data[0];
	unsigned char *time_stamp   = &data[0];
	unsigned char *size_stamp   = &data[0+8];
	int64_t time_in;
	uint32_t size;
	memcpy(&time_in,time_stamp,sizeof(time_in));
	memcpy(&size,size_stamp,sizeof(size));

	if(time_in < time(0) || size != calculated_content_size)
		return false;
	// check signature
	unsigned char *given_signature = &data[0+8+4 + size];
	std::vector<char> real_signature(digest_size,0);
	md.append(signed_body,8+4+size);
	md.readout(&real_signature[0]);
	if(memcmp(given_signature,&real_signature[0],digest_size)!=0)
		return false;
	
	// return valid data
	unsigned char *real_content = &data[0+8+4];
	plain.assign(reinterpret_cast<char*>(real_content),size);
	if(timeout)
		*timeout = time_in;
	return true;
}

} // impl
} // sessions
} // cppcms
