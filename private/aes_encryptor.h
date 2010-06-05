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
#include <gcrypt.h>
#include "base_encryptor.h"

namespace cppcms {
namespace sessions {
namespace impl {

class CPPCMS_API aes_cipher : public base_encryptor {
	bool loaded_;
	gcry_cipher_hd_t hd_out;
	gcry_cipher_hd_t hd_in;
	struct aes_hdr {
		char salt[16];
		char md5[16];
	};
	void load();
public:
	virtual std::string encrypt(std::string const &plain,time_t timeout);
	virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout=NULL) ;
	aes_cipher(std::string key);
	~aes_cipher();
};

} // impl
} // sessions
} // cppcms


#endif

