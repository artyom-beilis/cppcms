///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_TOHEX_H
#define CPPCMS_IMPL_TOHEX_H
namespace cppcms {
	namespace impl {
		inline void tohex(void const *vptr,size_t len,char *out)
		{
			unsigned char const *p=static_cast<unsigned char const *>(vptr);
			while(len>0) {
				static char const table[17]="0123456789abcdef";
				unsigned char v=*p++;
				*out++ = table[(v >> 4) & 0xF];
				*out++ = table[(v & 0xF)];
				len--;
			}
			*out++ = '\0';
		}
	}
}
#endif
