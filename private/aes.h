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
#ifndef CPPCMS_PRIVATE_AES_H
#define CPPCMS_PRIVATE_AES_H

#include <memory>
#include <string>


namespace cppcms {
namespace impl {

	struct aes_api {
		typedef enum { 
			aes128 = 128,
			aes192 = 192,
			aes256 = 256 
		} aes_type;
	
		virtual ~aes_api()
		{
		}
		virtual aes_api *clone() const = 0;
		virtual void encrypt(void const *in,void *out,unsigned len) = 0;
		virtual void decrypt(void const *in,void *out,unsigned len) = 0;

		static std::auto_ptr<aes_api> create(aes_type type,std::string const &key);
	};

} // impl
	
} //cppcms


#endif
