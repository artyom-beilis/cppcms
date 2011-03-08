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
#include <cppcms/crypto.h>
#include <cppcms/config.h>
#ifdef CPPCMS_HAVE_GCRYPT
#  include <gcrypt.h>
#endif
#ifdef CPPCMS_HAVE_OPENSSL
#  include <openssl/sha.h>
#endif
#include <vector>
#include <booster/backtrace.h>
#include <booster/nowide/cstdio.h>
#include "md5.h"
#include "sha1.h"

#include <string.h>

namespace cppcms {
namespace crypto {
	namespace {
		class md5_digets : public message_digest {
		public:
			md5_digets()
			{
				impl::md5_init(&state_);
			}
			virtual unsigned digest_size() const
			{
				return 16;
			}
			virtual unsigned block_size() const
			{
				return 64;
			}
			virtual void append(void const *ptr,size_t size) 
			{
				impl::md5_append(&state_,reinterpret_cast<impl::md5_byte_t const *>(ptr),size);
			}
			virtual void readout(void *ptr)
			{
				impl::md5_finish(&state_,reinterpret_cast<impl::md5_byte_t *>(ptr));
				impl::md5_init(&state_);
			}
			virtual char const *name() const
			{
				return "md5";
			}
			virtual md5_digets *clone() const
			{
				return new md5_digets();
			}
			virtual ~md5_digets()
			{
				memset(&state_,0,sizeof(state_));
			}
		private:
			impl::md5_state_t state_;
		};
		class sha1_digets : public message_digest {
		public:
			sha1_digets()
			{
				state_.reset();
			}
			virtual unsigned digest_size() const
			{
				return 20;
			}
			virtual unsigned block_size() const
			{
				return 64;
			}
			virtual void append(void const *ptr,size_t size) 
			{
				state_.process_bytes(ptr,size);
			}
			virtual void readout(void *ptr)
			{
				unsigned digets[5];
				state_.get_digest(digets);
				state_.reset();
				unsigned char *out = reinterpret_cast<unsigned char *>(ptr);
				for(unsigned i=0;i<5;i++) {
					unsigned block = digets[i];
					*out ++ = (block >> 24u) & 0xFFu;
					*out ++ = (block >> 16u) & 0xFFu;
					*out ++ = (block >>  8u) & 0xFFu;
					*out ++ = (block >>  0u) & 0xFFu;
				}
			}
			virtual char const *name() const
			{
				return "sha1";
			}
			virtual sha1_digets *clone() const
			{
				return new sha1_digets();
			}
			virtual ~sha1_digets()
			{
				memset(&state_,0,sizeof(state_));
			}
		private:
			impl::sha1 state_;
		};

		#ifdef CPPCMS_HAVE_GCRYPT
		class gcrypt_digets : public message_digest {
		public:
			gcrypt_digets(int algorithm)
			{
				if(gcry_md_open(&state_,algorithm,0)!=0)
					throw booster::invalid_argument("gcrypt_digets - faled to create md");
				algo_ = algorithm;
			}
			virtual unsigned digest_size() const
			{
				return gcry_md_get_algo_dlen(algo_);
			}
			virtual unsigned block_size() const
			{
				if(algo_ == GCRY_MD_SHA384 || algo_ == GCRY_MD_SHA512)
					return 128;
				else
					return 64;
			}
			virtual void append(void const *ptr,size_t size) 
			{
				gcry_md_write(state_,ptr,size);
			}
			virtual void readout(void *ptr)
			{
				memcpy(ptr,gcry_md_read(state_,0),digest_size());
				gcry_md_reset(state_);
			}
			virtual char const *name() const
			{
				switch(algo_) {
				case GCRY_MD_SHA224:
					return "sha224";
				case GCRY_MD_SHA256:
					return "sha256";
				case GCRY_MD_SHA384:
					return "sha384";
				case GCRY_MD_SHA512:
					return "sha512";
				default:
					return "unknown";
				}
			}
			virtual gcrypt_digets *clone() const
			{
				return new gcrypt_digets(algo_);
			}
			virtual ~gcrypt_digets()
			{
				gcry_md_close(state_);
				state_ = 0;
			}
		private:
			gcry_md_hd_t state_;
			int algo_;
		};

		#endif

		#ifdef CPPCMS_HAVE_OPENSSL
		
		#define CPPCMS_SSL_MD(len,base_len)				\
		class ssl_sha##len : public message_digest {			\
		public:								\
			ssl_sha##len() 						\
			{							\
				SHA ## len ## _Init(&state_);			\
			}							\
			virtual message_digest *clone() const			\
			{							\
				return new ssl_sha##len();			\
			}							\
			virtual ~ssl_sha##len()					\
			{							\
				memset(&state_,0,sizeof(state_));		\
			}							\
			virtual void append(void const *ptr,size_t size) 	\
			{							\
				SHA ## len ## _Update(&state_,ptr,size);	\
			}							\
			virtual void readout(void *ptr)				\
			{							\
				SHA ## len ## _Final(				\
					(unsigned char *)(ptr),			\
					&state_					\
					);					\
				SHA ## len ## _Init(&state_);			\
			}							\
			virtual char const *name() const			\
			{							\
				return "sha" #len;				\
			}							\
			virtual unsigned digest_size() const			\
			{							\
				return len / 8;					\
			}							\
			virtual unsigned block_size() const			\
			{							\
				if (len >= 384)					\
					return 128;				\
				else						\
					return 64;				\
			}							\
		private:							\
			SHA ## base_len ##	_CTX state_;			\
		};

		CPPCMS_SSL_MD(224,256)
		CPPCMS_SSL_MD(256,256)
		CPPCMS_SSL_MD(384,512)
		CPPCMS_SSL_MD(512,512)

		#endif

	} // anon
	
	std::auto_ptr<message_digest> message_digest::md5()
	{
		std::auto_ptr<message_digest> d(new md5_digets());
		return d;
	}

	std::auto_ptr<message_digest> message_digest::sha1()
	{
		std::auto_ptr<message_digest> d(new sha1_digets());
		return d;
	}

	std::auto_ptr<message_digest> message_digest::create_by_name(std::string const &namein)
	{
		std::auto_ptr<message_digest> d;
		std::string name = namein;
		for(unsigned i=0;i<name.size();i++)
			if('A' <= name[i] && name[i]<='Z')
				name[i] = name[i] - 'A' + 'a';
		if(name=="md5")
			d = md5();
		else if(name == "sha1")
			d = sha1();
		#ifdef CPPCMS_HAVE_GCRYPT
		else if(name == "sha224")
			d.reset(new gcrypt_digets(GCRY_MD_SHA224));
		else if(name == "sha256")
			d.reset(new gcrypt_digets(GCRY_MD_SHA256));
		else if(name == "sha384")
			d.reset(new gcrypt_digets(GCRY_MD_SHA384));
		else if(name == "sha512")
			d.reset(new gcrypt_digets(GCRY_MD_SHA512));
		#endif

		#ifdef CPPCMS_HAVE_OPENSSL
		else if(name == "sha224")
			d.reset(new ssl_sha224());
		else if(name == "sha256")
			d.reset(new ssl_sha256());
		else if(name == "sha384")
			d.reset(new ssl_sha384());
		else if(name == "sha512")
			d.reset(new ssl_sha512());
		#endif
		
		return d;
	}

	key::key() : data_(0), size_(0)
	{
	}
	
	key::key(key const &other) : data_(0),size_(0)
	{
		set(other.data(),other.size());
	}

	key const &key::operator=(key const &other)
	{
		if(this != &other) {
			set(other.data_,other.size_);
		}
		return *this;
	}

	key::key(void const *ptr,size_t len) : data_(0), size_(0)
	{
		set(ptr,len);
	}

	key::key(char const *s) : data_(0), size_(0)
	{
		if(s!=0)
			set_hex(s,strlen(s));
	}

	key::key(std::string const &s) : data_(0), size_(0)
	{
		set_hex(s.c_str(),s.size());
	}
	
	unsigned key::from_hex(char c)
	{
		if('0' <= c && c<='9') 
			return c-'0';
		if('a' <= c && c<='f')
			return c-'a' + 10;
		if('A' <= c && c<='F')
			return c-'A' + 10;
		return 0;
	}
	void key::read_from_file(std::string const &file_name)
	{
		reset();

		FILE *f = 0;
		char *buf = 0;
		size_t buf_size = 0;
		
		try {
			f = booster::nowide::fopen(file_name.c_str(),"rb");
			if(!f) {
				throw booster::runtime_error("cppcms::crypto::key Failed to open file:" + file_name);
			}
			setbuf(f,0);
			fseek(f,0,SEEK_END);
			long size = ftell(f);
			if(size < 0) {
				throw booster::runtime_error("cppcms::crypto::key failed to get file size:" + file_name);
			}
			if(size == 0) {
				throw booster::runtime_error("cppcms::crypto::key file " + file_name + " is empty");
			}
			fseek(f,0,SEEK_SET);
			buf = new char[size]();
			buf_size = size;
			if(fread(buf,1,buf_size,f)!=buf_size) {
				throw booster::runtime_error("cppcms::crypto::key failed reading file " + file_name);
			}
			fclose(f);
			f=0;
			int i;
			for(i=buf_size-1;i>=0;i--) {
				if(buf[i]==' ' || buf[i]=='\n' || buf[i]=='\r' || buf[i]=='\t')
					continue;
				break;
			}
			size_t real_size = i + 1;
			set_hex(buf,real_size);
		}
		catch(...) {
			if(f) { 
				fclose(f);
				f=0;
			}
			if(buf) {
				memset(buf,0,buf_size);
				delete [] buf;
				buf = 0;
			}
			throw;
		}
		if(f) {
			fclose(f);
			f=0;
		}
		if(buf) {
			memset(buf,0,buf_size);
			delete [] buf;
			buf = 0;
		}
	}

	void key::set_hex(char const *ptr,size_t len)
	{
		reset();
		if(len == 0)
			return;
		if(len % 2 != 0) {
			throw booster::invalid_argument("cppcms::crypto::key: the hexadecimal key length is not multiple of 2");
		}
		for(unsigned i=0;i<len;i++) {
			char c=ptr[i];
			if(	('0'<=c && c<='9')
				|| ('a' <= c && c<='f')
				|| ('A' <= c && c<='F')
			  )
			{
				continue;
			}
			throw booster::invalid_argument("cppcms::crypto::key: the hexadecimal key has invalid characters");
		}
		size_ = len / 2;
		data_ = new char[size_];
		for(unsigned h=0,b=0;h<len;h+=2,b++) {
			data_[b] = (from_hex(ptr[h]) << 4) + from_hex(ptr[h+1]);
		}
	}

	void key::set(void const *ptr,size_t len)
	{
		reset();
		if(ptr) {
			data_ = new char[len];
			size_ = len;
			memcpy(data_,ptr,len);
		}
	}
	void key::reset()
	{
		if(data_) {
			memset(data_,0,size_);
			delete [] data_;
			data_ = 0;
			size_ = 0;
		}
	}
	key::~key()
	{
		reset();
	}

	char const *key::data() const
	{
		if(data_ == 0)
			return "";
		return data_;
	}
	size_t key::size() const
	{
		return size_;
	}

	struct hmac::data_ {};

	hmac::hmac(std::auto_ptr<message_digest> digest,key const &k) : key_(k)
	{
		if(!digest.get())
			throw booster::invalid_argument("Has algorithm is not provided");
		md_ = digest;
		md_opad_.reset(md_->clone());
		init();
	}
	hmac::hmac(std::string const &name,key const &k) : key_(k)
	{
		md_ = message_digest::create_by_name(name);
		if(!md_.get()) {
			throw booster::invalid_argument("Invalid or unsupported hash function:" + name);
		}
		md_opad_.reset(md_->clone());
		init();
	}
	hmac::~hmac()
	{
	}
	void hmac::init()
	{
		unsigned const block_size = md_->block_size();
		std::vector<unsigned char> ipad(block_size,0);
		std::vector<unsigned char> opad(block_size,0);
		if(key_.size() > block_size) {
			md_->append(key_.data(),key_.size());
			md_->readout(&ipad.front());
			memcpy(&opad.front(),&ipad.front(),md_->digest_size());
		}
		else {
			memcpy(&ipad.front(),key_.data(),key_.size());
			memcpy(&opad.front(),key_.data(),key_.size());
		}
		for(unsigned i=0;i<block_size;i++) {
			ipad[i]^=0x36 ;
			opad[i]^=0x5c ;
		}
		md_opad_->append(&opad.front(),block_size);
		md_->append(&ipad.front(),block_size);
		ipad.assign(block_size,0);
		opad.assign(block_size,0);
	}
	unsigned hmac::digest_size() const
	{
		if(!md_.get()){
			throw booster::runtime_error("Hmac can be used only once");
		}
		return md_->digest_size();
	}
	void hmac::append(void const *ptr,size_t size)
	{
		if(!md_.get()){
			throw booster::runtime_error("Hmac can be used only once");
		}
		md_->append(ptr,size);
	}
	void hmac::readout(void *ptr)
	{
		std::vector<unsigned char> digest(md_->digest_size(),0);
		md_->readout(&digest.front()); // md_ is reset
		md_opad_->append(&digest.front(),md_->digest_size());
		md_opad_->readout(ptr); // md_opad_ is reset now
		digest.assign(md_->digest_size(),0);
		init();
	}

} // crypto
} // cppcms
