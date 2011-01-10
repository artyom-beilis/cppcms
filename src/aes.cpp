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
#include <cppcms/urandom.h>
#include <cppcms/crypto.h>
#include <cppcms/config.h>
#include <booster/backtrace.h>
#include <string.h>


#if defined(CPPCMS_HAVE_OPENSSL)
	#include <openssl/aes.h>
	#define CPPCMS_HAVE_AES
#endif

#if defined(CPPCMS_HAVE_GCRYPT)
	#include <gcrypt.h>
	#define CPPCMS_HAVE_AES

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
namespace crypto {

#if defined CPPCMS_HAVE_GCRYPT

	class gcrypt_aes_encryptor : public cbc {
	public:
		gcrypt_aes_encryptor(unsigned type) : 
			enc_(0),
			dec_(0),
			iv_initialized_(false)
		{
			memset(iv_,0,sizeof(iv_));
			type_ = type;
		}

		void check()
		{
			if(key_.size() == 0)
				throw booster::runtime_error("cppcms::crypto::aes: attempt to use cbc without key");
			if(!iv_initialized_)
				throw booster::runtime_error("cppcms::crypto::aes: attempt to use cbc without initial vector set");
		}
		unsigned block_size() const
		{
			return 16;
		}
		unsigned key_size() const
		{
			return type_ / 8;
		}
		void set_key(key const &k)
		{
			if(key_.size()!=0)
				booster::runtime_error("cppcms::crypto::aes can't set key more then once");
			if(k.size() != key_size())
				throw booster::invalid_argument("cppcms::crypto::aes Invalid key size");
			key_ = k;
		}
		void set_iv(void const *ptr,size_t size)
		{
			if(size != sizeof(iv_))
				throw booster::invalid_argument("cppcms::crypto::aes: Invalid IV size");
			memcpy(iv_,ptr,size);
			iv_initialized_ = true;
			reset_iv();
		}
		void set_nonce_iv()
		{
			urandom_device rnd;
			rnd.generate(iv_,sizeof(iv_));
			iv_initialized_ = true;
			reset_iv();
		}

		void reset_iv()
		{
			if(enc_) {
				if(gcry_cipher_setiv(enc_,iv_,16)) {
					throw booster::runtime_error("cppcms::crypto::aes: failed to reset iv");
				}
			}
			if(dec_) {
				if(gcry_cipher_setiv(dec_,iv_,16)) {
					throw booster::runtime_error("cppcms::crypto::aes: failed to reset iv");
				}
			}
		}


		virtual ~gcrypt_aes_encryptor()
		{
			if(enc_)
				gcry_cipher_close(enc_);
			if(dec_)
				gcry_cipher_close(dec_);
			memset(iv_,0,sizeof(iv_));
		}
		virtual void encrypt(void const *in,void *out,unsigned len)
		{
			init(enc_);
			if(gcry_cipher_encrypt(enc_,out,len,in,len)!=0) {
				throw booster::runtime_error("Encryption failed");
			}
		}
		virtual void decrypt(void const *in,void *out,unsigned len)
		{
			init(dec_);
			if(gcry_cipher_decrypt(dec_,out,len,in,len)!=0) {
				throw booster::runtime_error("Decryption failed");
			}
		}
	private:
		void init(gcry_cipher_hd_t &h)
		{
			if(h)
				return;
			check();

			int algo=0;
			switch(type_) {
			case 128:
				algo = GCRY_CIPHER_AES128;
				break;
			case 192:
				algo = GCRY_CIPHER_AES192;
				break;
			case 256:
				algo = GCRY_CIPHER_AES256;
				break;
			default:
				throw booster::invalid_argument("Invalid encryption method");
			}

			if(	gcry_cipher_open(&h,algo,GCRY_CIPHER_MODE_CBC,0)
				|| gcry_cipher_setkey(h,key_.data(),key_.size())
				|| gcry_cipher_setiv(h,iv_,16)
			  )
			{
				if(h) {
					gcry_cipher_close(h);
					h=0;
				}
				throw booster::runtime_error("Failed to create AES encryptor");
			}
		}

		gcry_cipher_hd_t enc_;
		gcry_cipher_hd_t dec_;
		key key_;
		char iv_[16];
		bool iv_initialized_;
		unsigned type_;
	};

	typedef gcrypt_aes_encryptor aes_encryption_provider;

#elif defined CPPCMS_HAVE_OPENSSL

	class openssl_aes_encryptor : public cbc {
	public:
		openssl_aes_encryptor(unsigned bits)
		{
			switch(bits) {
			case 128:
			case 192:
			case 256:
				type_  = bits;
				break;
			default:
				throw booster::invalid_argument("cppcms::crypto::aes invalid algorithm");
			}
			reset();
		}
		void reset()
		{
			key_.reset();
			memset(&key_enc_,0,sizeof(key_enc_));
			memset(&key_dec_,0,sizeof(key_dec_));
			memset(iv_dec_,0,sizeof(iv_dec_));
			memset(iv_enc_,0,sizeof(iv_dec_));
			encryption_initialized_ = false;
			decryption_initialized_ = false;
			iv_initialized_ = false;
		}
		void check()
		{
			if(key_.size() == 0)
				throw booster::runtime_error("cppcms::crypto::aes: attempt to use cbc without key");
			if(!iv_initialized_)
				throw booster::runtime_error("cppcms::crypto::aes: attempt to use cbc without initial vector set");
		}
		unsigned block_size() const
		{
			return 16;
		}
		unsigned key_size() const
		{
			return type_ / 8;
		}
		void set_key(key const &k)
		{
			if(key_.size()!=0)
				booster::runtime_error("cppcms::crypto::aes can't set key more then once");
			if(k.size() != key_size())
				throw booster::invalid_argument("cppcms::crypto::aes Invalid key size");
			key_ = k;
		}
		void set_iv(void const *ptr,size_t size)
		{
			if(size != sizeof(iv_enc_))
				throw booster::invalid_argument("cppcms::crypto::aes: Invalid IV size");
			memcpy(iv_enc_,ptr,size);
			memcpy(iv_dec_,ptr,size);
			iv_initialized_ = true;
		}
		void set_nonce_iv()
		{
			urandom_device rnd;
			rnd.generate(iv_enc_,sizeof(iv_enc_));
			rnd.generate(iv_dec_,sizeof(iv_dec_));
			iv_initialized_ = true;
		}
		virtual void encrypt(void const *in,void *out,unsigned len)
		{
			check();
			if(!encryption_initialized_) {
				AES_set_encrypt_key(reinterpret_cast<unsigned char const *>(key_.data()), type_, &key_enc_);
				encryption_initialized_ = true;
			}
			AES_cbc_encrypt(reinterpret_cast<unsigned char const *>(in),
					reinterpret_cast<unsigned char *>(out), len,
					&key_enc_,
					iv_enc_,
					AES_ENCRYPT);
		}
		virtual void decrypt(void const *in,void *out,unsigned len)
		{
			check();
			if(!decryption_initialized_) {
				AES_set_decrypt_key(reinterpret_cast<unsigned char const *>(key_.data()), type_, &key_dec_);
				decryption_initialized_ = true;
			}

			AES_cbc_encrypt(reinterpret_cast<unsigned char const *>(in),
					reinterpret_cast<unsigned char *>(out), len,
					&key_dec_,
					iv_dec_,
					AES_DECRYPT);
		}
		virtual ~openssl_aes_encryptor()
		{
			reset();
		}
	private:
		key key_;
		unsigned type_;
		AES_KEY key_enc_;
		AES_KEY key_dec_;
		unsigned char iv_enc_[16];
		unsigned char iv_dec_[16];
		bool encryption_initialized_;
		bool decryption_initialized_;
		bool iv_initialized_;
	};

	typedef openssl_aes_encryptor aes_encryption_provider;

#endif

std::auto_ptr<cbc> cbc::create(std::string const &name)
{
	std::auto_ptr<cbc> res;
	if(name=="aes" || name=="AES" || name=="aes128" || name=="aes-128" || name=="AES128" || name=="AES-128")
		res = cbc::create(aes128);
	else if(name=="aes192" || name=="aes-192" || name=="AES192" || name=="AES-192")
		res = cbc::create(aes192);
	else if(name=="aes256" || name=="aes-256" || name=="AES256" || name=="AES-256")
		res = cbc::create(aes256);

	return res;
}

std::auto_ptr<cbc> cbc::create(cbc::cbc_type type)
{
	std::auto_ptr<cbc> res;
	switch(type) {
#ifdef CPPCMS_HAVE_AES
	case aes128:
		res.reset(new aes_encryption_provider(128));
		break;
	case aes192:
		res.reset(new aes_encryption_provider(192));
		break;
	case aes256:
		res.reset(new aes_encryption_provider(256));
		break;
#endif
	default:
		;
	}
	return res;
}


} // crypto
} //cppcms


