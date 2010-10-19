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
#include <cppcms/cppcms_error.h>

#ifdef CPPCMS_WIN_NATIVE

#include <sstream>
#include <booster/thread.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows.h"
#include "wincrypt.h"

namespace cppcms {


	class urandom_device_impl {
	public:
		urandom_device_impl() {
			if(CryptAcquireContext(&provider_,0,0,PROV_RSA_FULL,0)) 
				return;
			if(GetLastError() == NTE_BAD_KEYSET) {
				if(CryptAcquireContext(&provider_,0,0,PROV_RSA_FULL,CRYPT_NEWKEYSET))
					return;
			}
			
			std::ostringstream ss;
			ss<<"CryptAcquireContext failed with code 0x"<<std::hex<<GetLastError();
			throw cppcms_error(ss.str());
		}

		~urandom_device_impl()
		{
			CryptReleaseContext(provider_,0);
		}
		void generate(void *ptr,unsigned len)
		{
			if(CryptGenRandom(provider_,len,static_cast<BYTE *>(ptr)))
				return;
			std::ostringstream ss;
			ss<<"CryptGenRandom failed with code 0x"<<std::hex<<GetLastError();
			throw cppcms_error(ss.str());
		}
	private:
		HCRYPTPROV provider_;
	};
	
	booster::thread_specific_ptr<urandom_device_impl>  urandom_device_impl_ptr;

	struct urandom_device::_data {};

	urandom_device::urandom_device() 
	{
	}
	urandom_device::~urandom_device()
	{
	}
	void urandom_device::generate(void *ptr,unsigned len)
	{
		if(!urandom_device_impl_ptr.get())
			urandom_device_impl_ptr.reset(new urandom_device_impl());
		urandom_device_impl_ptr->generate(ptr,len);
	}
} // cppcms


#else

#ifndef CPPCMS_WIN32
#include "daemonize.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


namespace cppcms {
	struct urandom_device::_data {};

	urandom_device::urandom_device()
	{
	}
	urandom_device::~urandom_device()
	{
	}
	void urandom_device::generate(void *ptr,unsigned len)
	{
		if(len == 0)
			return;
		int n = 0;
		#ifndef CPPCMS_WIN32
		if(impl::daemonizer::global_urandom_fd!=-1) {
			n = read(impl::daemonizer::global_urandom_fd,ptr,len);
		}
		else 
		#endif
		{
			int fd = open("/dev/urandom",O_RDONLY);
			if(!fd) 
				throw cppcms_error("Failed to open /dev/urandom");
			n = read(fd,ptr,len);
			close(fd);
		}
		if(n!=int(len))
			throw cppcms_error("Failed to read /dev/urandom");
	}
}

#endif
