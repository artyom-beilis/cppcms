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
#define CPPCMS_SOURCE
#include "aes.h"
#include <cppcms/urandom.h>
#include <cppcms/config.h>
#include <booster/backtrace.h>
#include <string.h>


#if defined(CPPCMS_HAVE_OPENSSL)
	#include <openssl/aes.h>
#endif

#if defined(CPPCMS_HAVE_GCRYPT)
	#include <gcrypt.h>

	#ifdef CPPCMS_WIN_NATIVE
	#	define CPPCMS_GCRYPT_USE_BOOSTER_THREADS
	#endif

	#ifdef CPPCMS_GCRYPT_USE_BOOSTER_THREADS

		#include <booster/thread.h>

		extern "C" {
			static int nt_mutex_init(void **p)
			{
				try {
					*p=new booster::mutex();
					return 0;
				}
				catch(...)
				{
					return 1;
				}
			}
			static int nt_mutex_destroy(void **p)
			{
				booster::mutex **m=reinterpret_cast<booster::mutex **>(p);
				delete *m;
				*m=0;
				return 0;
			}

			static int nt_mutex_lock(void **p)
			{
				booster::mutex *m=reinterpret_cast<booster::mutex *>(*p);
				try {
					m->lock();
					return 0;
				}
				catch(...)
				{
					return 1;
				}
			}

			static int nt_mutex_unlock(void **p)
			{
				booster::mutex *m=reinterpret_cast<booster::mutex *>(*p);
				try {
					m->unlock();
					return 0;
				}
				catch(...)
				{
					return 1;
				}
			}

			static struct gcry_thread_cbs threads_nt = { 
				GCRY_THREAD_OPTION_USER,
				0,
				nt_mutex_init,
				nt_mutex_destroy,
				nt_mutex_lock,
				nt_mutex_unlock,
				0,0,0,0,
				0,0,0,0 
			};

			static void set_gcrypt_cbs()
			{
				gcry_control (GCRYCTL_SET_THREAD_CBS, &threads_nt);
			}
		} // extern "C"
						
	#else // pthreads

		#include <pthread.h>
		#include <errno.h>

		extern "C" {

			GCRY_THREAD_OPTION_PTHREAD_IMPL;
			
			static void set_gcrypt_cbs()
			{
				gcry_control (GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
			}
		} // exten C

	#endif

	// Static library initialization
	namespace {
		class load {
			public:
			load() {
				set_gcrypt_cbs();
				gcry_check_version(NULL);
			}
		} loader;
	} // anon namespace

#endif // defined(CPPCMS_HAVE_GCRYPT)



namespace cppcms {
namespace impl {

#if defined CPPCMS_HAVE_GCRYPT

	class gcrypt_aes_encryptor : public aes_api {
		void operator=(gcrypt_aes_encryptor const &);
	public:
		gcrypt_aes_encryptor(gcrypt_aes_encryptor const &other)
		{
			init(other.type_,other.key_);
		}
		gcrypt_aes_encryptor(aes_type type,std::string const &key)
		{
			init(type,key);
		}
		virtual ~gcrypt_aes_encryptor()
		{
			gcry_cipher_close(enc_);
			gcry_cipher_close(dec_);
		}
		virtual gcrypt_aes_encryptor *clone() const
		{
			return new gcrypt_aes_encryptor(*this);
		}
		virtual void encrypt(void const *in,void *out,unsigned len)
		{
			if(len % 16 != 0) {
				throw booster::invalid_argument("Invalid block size");
			}
			if(gcry_cipher_encrypt(enc_,out,len,in,len)!=0) {
				throw booster::runtime_error("Encryption failed");
			}
		}
		virtual void decrypt(void const *in,void *out,unsigned len)
		{
			if(len % 16 != 0) {
				throw booster::invalid_argument("Invalid block size");
			}
			if(gcry_cipher_decrypt(enc_,out,len,in,len)!=0) {
				throw booster::runtime_error("Decryption failed");
			}
		}
	private:
		void init(aes_type type,std::string const &key)
		{
			if(key.size()*8!=unsigned(type)) {
				throw booster::invalid_argument("Invalid key size");
			}
			key_ = key;
			type_ = type;
			int algo=0;
			switch(type) {
			case aes128:
				algo = GCRY_CIPHER_AES128;
				break;
			case aes192:
				algo = GCRY_CIPHER_AES192;
				break;
			case aes256:
				algo = GCRY_CIPHER_AES256;
				break;
			default:
				throw booster::invalid_argument("Invalid encryption method");
			}

			enc_ = 0;
			dec_ = 0;
			char iv_enc[16];
			char iv_dec[16]={0};
			urandom_device rnd;
			rnd.generate(iv_enc,sizeof(iv_enc));

			if(	gcry_cipher_open(&enc_,algo,GCRY_CIPHER_MODE_CBC,0)
				|| gcry_cipher_open(&dec_,algo,GCRY_CIPHER_MODE_CBC,0)
				|| gcry_cipher_setkey(enc_,key_.c_str(),key.size())
				|| gcry_cipher_setkey(dec_,key_.c_str(),key.size())
				|| gcry_cipher_setiv(enc_,iv_enc,16)
				|| gcry_cipher_setiv(dec_,iv_dec,16)
			  )
			{
				if(enc_)
					gcry_cipher_close(enc_);
				if(dec_)
					gcry_cipher_close(dec_);
				throw booster::runtime_error("Failed to create AES encryptor");
			}

		}

		gcry_cipher_hd_t enc_;
		gcry_cipher_hd_t dec_;
		std::string key_;
		aes_type type_;

	};
	typedef gcrypt_aes_encryptor aes_encryption_provider;

#elif defined CPPCMS_HAVE_OPENSSL

	class openssl_aes_encryptor : public aes_api {
		void operator=(openssl_aes_encryptor const &);
	public:
		openssl_aes_encryptor(openssl_aes_encryptor const &other) :
			key_enc_(other.key_enc_),
			key_dec_(other.key_dec_)
		{
			reset();
		}

		openssl_aes_encryptor(aes_type type,std::string const &key)
		{
			if(key.size()*8!=unsigned(type)) {
				throw booster::invalid_argument("Invalid key size");
			}
			AES_set_encrypt_key(reinterpret_cast<unsigned char const *>(key.c_str()), type, &key_enc_);
			AES_set_decrypt_key(reinterpret_cast<unsigned char const *>(key.c_str()), type, &key_dec_);
			
			reset();
		}
		virtual openssl_aes_encryptor *clone() const
		{
			return new openssl_aes_encryptor(*this);
		}
		virtual void encrypt(void const *in,void *out,unsigned len)
		{
			if(len % 16 != 0) {
				throw booster::invalid_argument("Invalid block size");
			}
			AES_cbc_encrypt(reinterpret_cast<unsigned char const *>(in),
					reinterpret_cast<unsigned char *>(out), len,
					&key_enc_,
					iv_enc_,
					AES_ENCRYPT);
		}
		virtual void decrypt(void const *in,void *out,unsigned len)
		{
			if(len % 16 != 0) {
				throw booster::invalid_argument("Invalid block size");
			}
			AES_cbc_encrypt(reinterpret_cast<unsigned char const *>(in),
					reinterpret_cast<unsigned char *>(out), len,
					&key_dec_,
					iv_dec_,
					AES_DECRYPT);
		}
		virtual ~openssl_aes_encryptor()
		{
			memset(&key_enc_,0,sizeof(key_enc_));
			memset(&key_dec_,0,sizeof(key_dec_));
			memset(iv_dec_,0,sizeof(iv_dec_));
			memset(iv_enc_,0,sizeof(iv_dec_));
		}
	private:
		void reset()
		{
			memset(iv_dec_,0,sizeof(iv_dec_));
			urandom_device rnd;
			rnd.generate(iv_enc_,sizeof(iv_enc_));
		}

		AES_KEY key_enc_;
		AES_KEY key_dec_;
		unsigned char iv_enc_[16];
		unsigned char iv_dec_[16];
	};

	typedef openssl_aes_encryptor aes_encryption_provider;

#else
# error "Invalid conditions"
#endif

std::auto_ptr<aes_api> aes_api::create(aes_api::aes_type type,std::string const &key)
{
	std::auto_ptr<aes_api> res(new aes_encryption_provider(type,key));
	return res;
}


} // impl
} //cppcms


