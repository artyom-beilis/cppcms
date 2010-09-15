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
#ifndef CPPCMS_ENCRYPTOR_H
#define CPPCMS_ENCRYPTOR_H

#include <vector>
#include <string>

#include <cppcms/session_cookies.h>
#include <cppcms/urandom.h>
#include <cppcms/cstdint.h>

namespace cppcms {
namespace sessions {
namespace impl {

class CPPCMS_API base_encryptor : public cppcms::sessions::encryptor {
public:
	virtual std::string encrypt(std::string const &plain,time_t timeout) = 0;
	virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout=NULL) = 0;
	virtual ~base_encryptor();
protected:
	static std::string to_binary(std::string const &hex);
	std::string base64_enc(std::vector<unsigned char> const &data);
	void base64_dec(std::string const &,std::vector<unsigned char> &data);
	struct info {
		int64_t timeout;
		uint16_t size;
		char salt[6];
	};
private:
};

} // impl
} // sessions
} // cppcms

#endif
