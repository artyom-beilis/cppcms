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
#include "base_encryptor.h"
#include "aes.h"

namespace cppcms {
namespace sessions {
namespace impl {

class CPPCMS_API aes_cipher : public base_encryptor {
public:
	aes_cipher(std::string key,std::string name);
	~aes_cipher();
	virtual std::string encrypt(std::string const &plain,time_t timeout);
	virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout=NULL) ;
private:
	struct aes_hdr {
		char salt[16];
		char md5[16];
	};
	void load();
	std::auto_ptr<cppcms::impl::aes_api> api_;
	cppcms::impl::aes_api::aes_type type_;
	std::string key_;

};

} // impl
} // sessions
} // cppcms


#endif

