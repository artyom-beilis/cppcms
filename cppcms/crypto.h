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
	/// \brief This namespace holds basic cryptographic utilities useful for save interaction with user.
	///
	/// One of the limitations of these functions is the fact that they do not use so called cryptographic memory,
	/// however it is not required as all secret keys are stored as plain text in configuration files,
	/// The only protection that is given is that values of keys are erased when the object is deleted
	/// to prevent key-leaks due to use of uninitialized memory in user applications.
	///
	///
	namespace crypto {
		///
		/// \brief Key object, holds the string that represents the binary key.
		///
		/// When the key is destroyed it zeros all the memory it uses to prevent accidental
		/// leaks of the highly confidential data
		///
		class key {
		public:
			///
			/// Create an empty key on 0 length
			///
			key();
			///
			/// Copy the key
			///
			key(key const &other);
			///
			/// Assign the key
			///
			key const &operator=(key const &);
			///
			/// Destroy they key object clearing the area used by it.
			///
			~key();
			///
			/// Create a key using binary representation pointed by \a data of \a length bytes
			///
			key(void const *data,size_t length);
			///
			/// Create a key using the hexadecimal representation
			///
			explicit key(char const *s);
			///
			/// Create a key using the hexadecimal representation
			///
			explicit key(std::string const &);
			///
			/// Get the pointer to data, never returns NULL even if size() == 0
			///
			char const *data() const;
			///
			/// Get the size of the key
			///
			size_t size() const;

			///
			/// Clear the key - and clear the area it was stored it
			///
			void reset();

			///
			/// Set the binary value for the key
			///
			void set(void const *ptr,size_t len);
			///
			/// Set the value for the key using hexadecimal representation 
			///
			void set_hex(char const *ptr,size_t len);

		private:
			static unsigned from_hex(char c);
			char *data_;
			size_t size_;
		};
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
			hmac(std::auto_ptr<message_digest> digest,key const &k);
			///
			/// Create hmac that uses message digest algorithm called \a name and use a binary key - \a key
			///
			hmac(std::string const &name,key const &k);
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
			/// the state of the hmac is reset and it can be used for signing again
			///
			/// Note: provided buffer must be digest_size() bytes long.
			///
			void readout(void *ptr);
		private:
			void init();	
			struct data_;
			booster::hold_ptr<data_> d;
			std::auto_ptr<message_digest> md_,md_opad_;
			key key_;
		};

		///
		/// \brief  Cipher-block chaining encryption and decryption cryptographic service.
		/// 
		/// \note In order to use it, you \b must compile CppCMS with OpenSSL (libcrypto) or GNU-TLS (libgcrypt) library.
		///
		class CPPCMS_API cbc  : public booster::noncopyable {
		public:
			///
			/// CBC encryption type
			///
			typedef enum {
				aes128 	= 0,	///< AES-128
				aes192	= 1,	///< AES-192
				aes256	= 2 	///< AES-256
			} cbc_type;
		
			///
			/// Create a new cbc object that performs encryption using \a type method.
			///
			/// If the encryption method is not supported returns an empty pointer! 
			///
			static std::auto_ptr<cbc> create(cbc_type type);
			///
			/// Create a new cbc object that performs encryption using algorithm \a name
			///
			/// If the encryption method is not supported returns an empty pointer! 
			///
			/// Currently supported aes128, aes192, aes256, with names "aes" = "aes-128" = "aes128" , "aes-192" "aes192",
			/// "aes-256" = "aes256". They require CppCMS to be compiled with OpenSSL or GNU-TLS library
			///
			static std::auto_ptr<cbc> create(std::string const &name);

			///
			/// Get the size of the block CBC works on
			///
			virtual unsigned block_size() const = 0;
			///
			/// Get the required key size in bytes
			///
			virtual unsigned key_size() const = 0;

			///
			/// Set the key value
			///
			virtual void set_key(key const &) = 0;
			///
			/// Set initial vector value, size should be equal  to block_size()
			///
			virtual void set_iv(void const *ptr,size_t size) = 0;
			///
			/// Set randomly created initial vector value
			///
			virtual void set_nonce_iv() = 0;
			///
			/// Encrypt the data \a in to \a out of size \a len. \a len should be multiple of block_size()
			///
			virtual void encrypt(void const *in,void *out,unsigned len) = 0;
			///
			/// Decrypt the data \a in to \a out of size \a len. \a len should be multiple of block_size()
			///
			virtual void decrypt(void const *in,void *out,unsigned len) = 0;

			virtual ~cbc() 
			{
			}
			
		};

	} // crypto

} // cppcms



#endif
