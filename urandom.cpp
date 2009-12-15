#define CPPCMS_SOURCE
#include "urandom.h"
#include "cppcms_error.h"

#ifdef CPPCMS_WIN_NATIVE

#include "windows.h"
#include "wincrypt.h"
#include <sstream>

namespace cppcms {

	struct urandom_device::data {
		HCRYPTPROV provider;
		data() : provider(0) {}
	};

	urandom_device::urandom_device() :
		d(new data())
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
	struct urandom_device::data {};

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
