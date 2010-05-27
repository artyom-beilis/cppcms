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

#include "windows.h"
#include "wincrypt.h"
#include <sstream>

namespace cppcms {

	struct urandom_device::_data {
		HCRYPTPROV provider;
		data() : provider(0) {}
	};

	urandom_device::urandom_device() :
		d(new _data())
	{
		if(CryptAcquireContext(&d->provider,0,0,PROV_RSA_FULL,0)) 
			return;
		if(GetLastError() == NTE_BAD_KEYSET) {
			if(CryptAcquireContext(&d->provider,0,0,PROV_RSA_FULL,CRYPT_NEWKEYSET))
				return;
		}
		
		std::ostringstream ss;
		ss<<"CryptAcquireContext failed with code 0x"<<std::hex<<GetLastError();
		throw cppcms_error(ss.str());

	}
	urandom_device::~urandom_device()
	{
		CryptReleaseContext(d->provider,0);
	}
	void urandom_device::generate(void *ptr,unsigned len)
	{
		if(CryptGenRandom(d->provider,len,static_cast<BYTE *>(ptr)))
			return;
		std::ostringstream ss;
		ss<<"CryptGenRandom failed with code 0x"<<std::hex<<GetLastError();
		throw cppcms_error(ss.str());
	}
}


#else

#include <fstream>

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
		std::ifstream u("/dev/urandom");
		if(u.good() && !u.read(reinterpret_cast<char *>(ptr),len).fail() && u.gcount()==int(len))
			return;
		throw cppcms_error("Failed to read /dev/urandom");
	}
}

#endif
