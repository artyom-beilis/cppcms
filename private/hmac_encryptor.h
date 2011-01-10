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
#ifndef CPPCMS_HMAC_ENCRYPTOR_H
#define CPPCMS_HMAC_ENCRYPTOR_H
#include <cppcms/session_cookies.h>
#include <cppcms/crypto.h>
#include <cppcms/config.h>
#include <booster/thread.h>

namespace cppcms {
namespace sessions {
namespace impl {

class CPPCMS_API hmac_factory : public encryptor_factory {
public:
	virtual std::auto_ptr<encryptor> get();
	hmac_factory(std::string const &algo,crypto::key const &k);
	virtual ~hmac_factory() {}
private:
	std::string algo_;
	crypto::key key_;
};

class CPPCMS_API hmac_cipher : public cppcms::sessions::encryptor {
public:
	hmac_cipher(std::string const &hash_name,crypto::key const &key);
	
	virtual std::string encrypt(std::string const &plain);
	virtual bool decrypt(std::string const &cipher,std::string &plain);
private:
	crypto::key key_;
	std::string hash_;
};

} // impl
} // sessions
} // cppcms


#endif
