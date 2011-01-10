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
#ifndef CPPCMS_AES_ENCRYPTOR_H
#define CPPCMS_AES_ENCRYPTOR_H

#include <string>
#include <cppcms/session_cookies.h>
#include <cppcms/crypto.h>

namespace cppcms {
namespace sessions {
namespace impl {

class CPPCMS_API aes_factory : public encryptor_factory {
public:
	aes_factory(std::string const &cbc,crypto::key const &cbc_key,std::string const &hmac,crypto::key const &hmac_key);
	aes_factory(std::string const &algo,crypto::key const &k);
	virtual std::auto_ptr<encryptor> get();
	virtual ~aes_factory() {}
private:
	std::string cbc_;
	crypto::key cbc_key_;
	std::string hmac_;
	crypto::key hmac_key_;
};

class CPPCMS_API aes_cipher : public cppcms::sessions::encryptor {
public:
	aes_cipher(std::string const &cbc_name,std::string const &md_name,crypto::key const &cbc_key,crypto::key const &md_key);
	~aes_cipher();
	virtual std::string encrypt(std::string const &plain);
	virtual bool decrypt(std::string const &cipher,std::string &plain);
private:
	void load();
	std::auto_ptr<crypto::cbc> cbc_;
	std::auto_ptr<crypto::message_digest> digest_;
	std::string cbc_name_,md_name_;
	crypto::key cbc_key_;
	crypto::key mac_key_;

};

} // impl
} // sessions
} // cppcms


#endif

