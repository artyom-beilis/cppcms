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
#ifndef CPPCMS_UTF_ITERATOR_H
#define CPPCMS_UTF_ITERATOR_H
#include <cppcms/cstdint.h>
#include <string.h>

namespace cppcms {

namespace utf {
	static const uint32_t illegal = 0xFFFFFFFFu;
	inline bool valid(uint32_t v)
	{
		if(v>0x10FFFF)
			return false;
		if(0xD800 <=v && v<= 0xDFFF) // surragates
			return false;
		return true;
	}
}

namespace utf8 {
        inline int trail_length(unsigned char c) 
        {
            if(c < 128)
                return 0;
            if(c < 194)
                return -1;
            if(c < 224)
                return 1;
            if(c < 240)
                return 2;
            if(c <=244)
                return 3;
            return -1;
        }
        inline int width(uint32_t value)
        {
            if(value <=0x7F) {
                return 1;
            }
            else if(value <=0x7FF) {
                return 2;
            }
            else if(value <=0xFFFF) {
                return 3;
            }
            else {
                return 4;
            }
        }

	// See RFC 3629
	// Based on: http://www.w3.org/International/questions/qa-forms-utf-8
	template<typename Iterator>
	uint32_t next(Iterator &p,Iterator e,bool html=false,bool /*decode*/=false)
	{
		using utf::illegal;
		if(p==e)
			return illegal;

		unsigned char lead = *p++;

		// First byte is fully validated here
		int trail_size = trail_length(lead);

		if(trail_size < 0)
			return illegal;

		//
		// Ok as only ASCII may be of size = 0
		// also optimize for ASCII text
		//
		if(trail_size == 0) {
			if(!html || (lead >= 0x20 && lead!=0x7F) || lead==0x9 || lead==0x0A || lead==0x0D)
				return lead;
			return illegal;
		}

		uint32_t c = lead & ((1<<(6-trail_size))-1);

		// Read the rest
		unsigned char tmp;
		switch(trail_size) {
			case 3:
				if(p==e)
					return illegal;
				tmp = *p++;
				c = (c << 6) | ( tmp & 0x3F);
			case 2:
				if(p==e)
					return illegal;
				tmp = *p++;
				c = (c << 6) | ( tmp & 0x3F);
			case 1:
				if(p==e)
					return illegal;
				tmp = *p++;
				c = (c << 6) | ( tmp & 0x3F);
		}

		// Check code point validity: no surrogates and
		// valid range
		if(!utf::valid(c))
			return illegal;

		// make sure it is the most compact representation
		if(width(c)!=trail_size + 1)
			return illegal;
		
		if(html && c<0xA0)
			return illegal;
		return c;
	} // valid


	template<typename Iterator>
	bool validate(Iterator p,Iterator e,size_t &count,bool html=false)
	{
		while(p!=e) {
			if(next(p,e,html)==utf::illegal)
				return false;
			count++;
		}
		return true;
	}
	
	template<typename Iterator>
	bool validate(Iterator p,Iterator e,bool html=false)
	{
		while(p!=e) 
			if(next(p,e,html)==utf::illegal)
				return false;
		return true;
	}


	struct seq {
		char c[4];
		unsigned len;
	};
	inline seq encode(uint32_t value)
	{
		seq out=seq();
		if(value <=0x7F) {
			out.c[0]=value;
			out.len=1;
		}
		else if(value <=0x7FF) {
			out.c[0]=(value >> 6) | 0xC0;
			out.c[1]=(value & 0x3F) | 0x80;
			out.len=2;
		}
		else if(value <=0xFFFF) {
			out.c[0]=(value >> 12) | 0xE0;
			out.c[1]=((value >> 6) & 0x3F) | 0x80;
			out.c[2]=(value & 0x3F) | 0x80;
			out.len=3;
		}
		else {
			out.c[0]=(value >> 18) | 0xF0;
			out.c[1]=((value >> 12) & 0x3F) | 0x80;
			out.c[2]=((value >> 6) & 0x3F) | 0x80;
			out.c[3]=(value & 0x3F) | 0x80;
			out.len=4;
		}
		return out;
	}
} // namespace utf8


namespace utf16 {

	// See RFC 2781
	inline bool is_first_surrogate(uint16_t x)
	{
		return 0xD800 <=x && x<= 0xDBFF;
	}
	inline bool is_second_surrogate(uint16_t x)
	{
		return 0xDC00 <=x && x<= 0xDFFF;
	}
	inline uint32_t combine_surrogate(uint16_t w1,uint16_t w2)
	{
		return ((uint32_t(w1 & 0x3FF) << 10) | (w2 & 0x3FF)) + 0x10000;
	}

	template<typename It>
	inline uint32_t next(It &current,It last)
	{
		uint16_t w1=*current++;
		if(w1 < 0xD800 || 0xDFFF < w1) {
			return w1;
		}
		if(w1 > 0xDBFF)
			return utf::illegal;
		if(current==last)
			return utf::illegal;
		uint16_t w2=*current++;
		if(w2 < 0xDC00 || 0xDFFF < w2)
			return utf::illegal;
		return combine_surrogate(w1,w2);
	}
	inline int width(uint32_t u)
	{
		return u>=0x100000 ? 2 : 1;
	}
	struct seq {
		uint16_t c[2];
		unsigned len;
	};
	inline seq encode(uint32_t u)
	{
		seq out=seq();
		if(u<=0xFFFF) {
			out.c[0]=u;
			out.len=1;
		}
		else {
			u-=0x10000;
			out.c[0]=0xD800 | (u>>10);
			out.c[1]=0xDC00 | (u & 0x3FF);
			out.len=2;
		}
		return out;
	}
} // utf16;

} // namespace cppcms



#endif
