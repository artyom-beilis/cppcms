///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_CRC32_H
#define CPPCMS_IMPL_CRC32_H
#include <cppcms/cstdint.h>
#include <zlib.h>

namespace cppcms {
namespace impl {

class crc32_calc {
public:
	crc32_calc() 
	{
		value_ = crc32(0,0,0);
	}
	void process_bytes(void const *ptr,size_t n)
	{
		if(n==0)
			return;
		value_ = crc32(value_,reinterpret_cast<Bytef const *>(ptr),n);
	}
	uint32_t checksum() const
	{
		return value_;
	}
private:
	uint32_t value_;
};

} // impl
} // cppcms
#endif
