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
	///
	/// \brief this class provides an API to calculate various cryptographic hash functions
	///
	class CPPCMS_API message_digest : public booster::noncopyable {
	protected:
		/// It should be implemented in derived classes
		message_digest()
		{
		}
	public:
		virtual ~message_digest()
		{
		}
		
		///
		/// Get the size of message digest, for example for MD5 it is 16, for SHA1 it is 20
		///
		virtual unsigned digest_size() const = 0;
		///
		/// Get processing block size, returns 64 or 128, used mostly for correct HMAC calculations
		///
		virtual unsigned block_size() const = 0;

		///
		/// Add more data of size bytes for processing
		///
		virtual void append(void const *ptr,size_t size) = 0;
		///
		/// Read the message digest for the data and reset it into initial state,
		/// provided buffer must be digest_size() bytes
		///
		virtual void readout(void *ptr) = 0;

		///
		/// Make a polymorphic copy of this object, note the state of copied object is reset to 
		/// initial
		///
		virtual message_digest *clone() const = 0;

		///
		/// Get the name of the hash function
		///
		virtual char const *name() const = 0;

		///
		/// Create MD5 message digest
		///
		static std::auto_ptr<message_digest> md5();
		///
		/// Create SHA1 message digest
		///
		static std::auto_ptr<message_digest> sha1();
		///
		/// Create message digest by name, more then sha1 and md5 may be supported,
		/// if CppCMS is compiled with cryptography library like libgcrypt or openssl
		///
		static std::auto_ptr<message_digest> create_by_name(std::string const &name);
	};
	
	///
	/// This object calculates the HMAC signature for the input data
	///
	class CPPCMS_API hmac : public booster::noncopyable  {
	public:
		///
		/// Create hmac that uses given \a digest algorithm and a binary key - \a key
		///
		hmac(std::auto_ptr<message_digest> digest,std::string const &key);
		///
		/// Create hmac that uses message digest algorithm called \a name and use a binary key - \a key
		///
		hmac(std::string const &name,std::string const &key);
		~hmac();

		///
		/// Get the size of the signtature
		///
		unsigned digest_size() const;

		///
		/// Add data for signing
		///
		void append(void const *ptr,size_t size);

		///
		/// Get the signature for all the data, after calling this function
		/// the state of the hmac is reset and it can't be used again for
		/// signing the data.
		///
		/// Note: provided buffer must be digest_size() bytes long.
		///
		void readout(void *ptr);
	private:
		void init(std::string const &);	
		struct data_;
		booster::hold_ptr<data_> d;
		std::auto_ptr<message_digest> md_,md_opad_;
	};

} // cppcms



#endif
