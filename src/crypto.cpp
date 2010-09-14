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
#include <cppcms/crypto.h>
#include <vector>
#include <stdexcept>
#include "md5.h"
#include "sha1.h"

#include <string.h>

namespace cppcms {
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

	std::auto_ptr<message_digest> message_digest::create_by_name(std::string const &name)
	{
		std::auto_ptr<message_digest> d;
		if(name=="md5" || name=="MD5")
			d = md5();
		else if(name == "sha1" || name=="SHA1")
			d = sha1();
		return d;
	}

	struct hmac::data_ {};

	hmac::hmac(std::auto_ptr<message_digest> digest,std::string const &key)
	{
		if(!digest.get())
			throw std::invalid_argument("Has algorithm is not provided");
		md_ = digest;
		md_opad_.reset(md_->clone());
		init(key);
	}
	hmac::hmac(std::string const &name,std::string const &key)
	{
		md_ = message_digest::create_by_name(name);
		if(!md_.get()) {
			throw std::invalid_argument("Invalid or unsupported hash function:" + name);
		}
		md_opad_.reset(md_->clone());
		init(key);
	}
	hmac::~hmac()
	{
	}
	void hmac::init(std::string const &skey)
	{
		unsigned const block_size = md_->block_size();
		std::vector<unsigned char> ipad(block_size,0);
		std::vector<unsigned char> opad(block_size,0);
		if(skey.size() > block_size) {
			md_->append(skey.c_str(),skey.size());
			md_->readout(&ipad.front());
			memcpy(&opad.front(),&ipad.front(),md_->digest_size());
		}
		else {
			memcpy(&ipad.front(),skey.c_str(),skey.size());
			memcpy(&opad.front(),skey.c_str(),skey.size());
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
			throw std::runtime_error("Hmac can be used only once");
		}
		return md_->digest_size();
	}
	void hmac::append(void const *ptr,size_t size)
	{
		if(!md_.get()){
			throw std::runtime_error("Hmac can be used only once");
		}
		md_->append(ptr,size);
	}
	void hmac::readout(void *ptr)
	{
		std::vector<unsigned char> digest(md_->digest_size(),0);
		md_->readout(&digest.front());
		md_opad_->append(&digest.front(),md_->digest_size());
		md_opad_->readout(ptr);
		digest.assign(md_->digest_size(),0);
		md_.reset();
		md_opad_.reset();
	}


} // cppcms
