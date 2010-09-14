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
#ifndef CPPCMS_CRYPTO_H
#define CPPCMS_CRYPTO_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <memory>
#include <string>

namespace cppcms {
	class CPPCMS_API message_digest : public booster::noncopyable {
	public:
		message_digest()
		{
		}
		virtual ~message_digest()
		{
		}
		
		virtual unsigned digest_size() const = 0;
		virtual unsigned block_size() const = 0;
		virtual void append(void const *ptr,size_t size) = 0;
		virtual void readout(void *ptr) = 0;
		virtual message_digest *clone() const = 0;
		virtual char const *name() const = 0;

		static std::auto_ptr<message_digest> md5();
		static std::auto_ptr<message_digest> sha1();
		static std::auto_ptr<message_digest> create_by_name(std::string const &name);
	};
	
	class CPPCMS_API hmac : public booster::noncopyable  {
	public:
		hmac(std::auto_ptr<message_digest> digest,std::string const &key);
		hmac(std::string const &name,std::string const &key);
		~hmac();
		unsigned digest_size() const;
		void append(void const *ptr,size_t size);
		void readout(void *ptr);
	private:
		void init(std::string const &);	
		struct data_;
		booster::hold_ptr<data_> d;
		std::auto_ptr<message_digest> md_,md_opad_;
	};

} // cppcms



#endif
