///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cppcms/cppcms_error.h>
#include "aes_encryptor.h"

#include <cppcms/base64.h>
#include <cppcms/crypto.h>
#include <string.h>
#include <time.h>

#include <cppcms/cstdint.h>

using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {

aes_factory::aes_factory(std::string const &cbc,crypto::key const &cbc_key,std::string const &hmac,crypto::key const &hmac_key) :
	cbc_(cbc),
	cbc_key_(cbc_key),
	hmac_(hmac),
	hmac_key_(hmac_key)
{
}
aes_factory::aes_factory(std::string const &algo,crypto::key const &k) :
	cbc_(algo),
	hmac_("sha1")
{
	std::auto_ptr<crypto::message_digest> md_ptr(crypto::message_digest::create_by_name(hmac_));
	std::auto_ptr<crypto::cbc> cbc_ptr(crypto::cbc::create(algo));
	if(!cbc_ptr.get()) {
		throw booster::invalid_argument("cppcms::sessions::aes_factory: the algorithm " + algo + " is not supported,"
						" or the cppcms library was compiled without OpenSSL/GNU-TLS support");
	}
	size_t digest_size = md_ptr->digest_size();
	size_t cbc_key_size = cbc_ptr->key_size();
	size_t expected_size = digest_size + cbc_key_size;

	if(k.size() == cbc_key_size + digest_size) {
		cbc_key_.set(k.data(),cbc_key_size);
		hmac_key_.set(k.data() + cbc_key_size,digest_size);
	}
	else if(k.size() >= cbc_key_size && cbc_key_size * 8 < 512) {
		std::string name = k.size() * 8 <= 256 ? "sha256" : "sha512";
		crypto::hmac mac(name,k);
		std::vector<char> k1(mac.digest_size(),0);		
		std::vector<char> k2(mac.digest_size(),0);
		mac.append("0",1);
		mac.readout(&k1[0]);
		mac.append("\1",1);
		mac.readout(&k2[0]);
		cbc_key_.set(&k1[0],cbc_key_size);
		hmac_key_.set(&k2[0],digest_size);
		memset(&k1[0],0,k1.size());
		memset(&k2[0],0,k2.size());
	}
	else {
		std::ostringstream ss;
		ss	<<"cppcms::sessions::aes_factory: invalid key length: " << k.size() << " bytes; " 
			<<"expected " << expected_size << " or at least: " << cbc_key_size << " bytes";
		throw booster::invalid_argument(ss.str());
	}
}


std::auto_ptr<encryptor> aes_factory::get()
{
	std::auto_ptr<encryptor> ptr;
	ptr.reset(new aes_cipher(cbc_,hmac_,cbc_key_,hmac_key_));
	return ptr;
}





aes_cipher::aes_cipher(std::string const &cbc_name,std::string const &md_name,crypto::key const &cbc_key,crypto::key const &md_key) :
	cbc_name_(cbc_name),
	md_name_(md_name),
	cbc_key_(cbc_key),
	mac_key_(md_key)
{
}

aes_cipher::~aes_cipher()
{
}

void aes_cipher::load()
{
	if(!cbc_.get()) {
		cbc_ = crypto::cbc::create(cbc_name_);
		if(!cbc_.get()) {
			throw booster::invalid_argument("cppcms::sessions::aes_cipher: the algorithm " + cbc_name_ + " is not supported,"
							" or the cppcms library was compiled without OpenSSL/GNU-TLS support");
		}
		cbc_->set_nonce_iv();
		cbc_->set_key(cbc_key_);
	}
	if(!digest_.get()) {
		digest_ = crypto::message_digest::create_by_name(md_name_);
		if(!digest_.get()) {
			throw booster::invalid_argument("cppcms::sessions::aes_cipher: the hash algorithm " + cbc_name_ + " is not supported,"
							" or the cppcms library was compiled without OpenSSL/GNU-TLS support");
		}
	}
}

std::string aes_cipher::encrypt(string const &plain)
{
	load();
	
	std::auto_ptr<crypto::message_digest> digest(digest_->clone());

	unsigned digest_size = digest->digest_size();
	uint32_t size = plain.size();
	size_t cbc_block_size = cbc_->block_size();

	size_t block_size=(size + sizeof(size) + (cbc_block_size-1)) / cbc_block_size*cbc_block_size 
		+ cbc_block_size;
		// 1st block is not in use due to iv
		// message length  bytes

	std::vector<char> input(block_size,0);
	std::vector<char> output(block_size + digest_size,0); // add sha1 hmac
	memcpy(&input[cbc_block_size],&size,sizeof(size));
	memcpy(&input[cbc_block_size + sizeof(size)],plain.c_str(),plain.size());
	
	cbc_->encrypt(&input.front(),&output.front(),block_size);
	crypto::hmac signature(digest,mac_key_);
	signature.append(&output[0],block_size);
	signature.readout(&output[block_size]);

	return std::string(&output[0],output.size());
}

bool aes_cipher::decrypt(std::string const &cipher,std::string &plain)
{
	load();
	size_t digest_size = digest_->digest_size();
	size_t block_size = cbc_->block_size();
	if(cipher.size() < digest_size + block_size) {
		return false;
	}
	size_t real_size = cipher.size() - digest_size;
	if(real_size % block_size != 0) {
		return false;
	}
	if(real_size / block_size < 2) {
		return false;
	}
	
	crypto::hmac signature(std::auto_ptr<crypto::message_digest>(digest_->clone()),mac_key_);
	signature.append(cipher.c_str(),real_size);
	std::vector<char> verify(digest_size,0);
	signature.readout(&verify[0]);

	if(memcmp(&verify[0],cipher.c_str() + real_size,digest_size)!=0) {
		memset(&verify[0],0,digest_size);
		return false;
	}
	
	std::vector<char> full_plain(real_size);
	cbc_->decrypt(cipher.c_str(),&full_plain[0],real_size);
	
	uint32_t size = 0;
	memcpy(&size,&full_plain[block_size],sizeof(size));
	if(size > real_size - block_size - sizeof(size)) {
		return false;
	}
	plain.assign(&full_plain[0] + block_size + sizeof(size),size);
	return true;
}


} // impl
} // sessions

} // cppcms


