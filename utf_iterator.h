#ifndef CPPCMS_UTF_ITERATOR_H
#define CPPCMS_UTF_ITERATOR_H
#include "cstdint.h"
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
	// See RFC 3629
	// Based on: http://www.w3.org/International/questions/qa-forms-utf-8
	template<typename Iterator>
	uint32_t next(Iterator &p,Iterator e,bool html=false,bool decode=false)
	{
		unsigned char c=*p++;
		unsigned char seq0,seq1=0,seq2=0,seq3=0;
		seq0=c;
		int len=1;
		if((c & 0xC0) == 0xC0) {
			if(p==e)
				return utf::illegal;
			seq1=*p++;
			len=2;
		}
		if((c & 0xE0) == 0xE0) {
			if(p==e)
				return utf::illegal;
			seq2=*p++;
			len=3;
		}
		if((c & 0xF0) == 0xF0) {
			if(p==e)
				return utf::illegal;
			seq3=*p++;
			len=4;
		}
		switch(len) {
		case 1: // ASCII -- remove codes for HTML only
			if(!html || seq0==0x9 || seq0==0x0A || seq0==0x0D || (0x20<=seq0 && seq0<=0x7E))
				break;
			return utf::illegal;
		case 2: // non-overloading 2 bytes
			if(0xC2 <= seq0 && seq0 <= 0xDF) {
				if(html && seq0==0xC2 && seq1<=0x9F)
					return utf::illegal; // C1 is illegal
				if(0x80 <= seq1 && seq1<= 0xBF)
					break;
			}
			return utf::illegal;
		case 3: 
			if(seq0==0xE0) { // exclude overloadings
				if(0xA0 <=seq1 && seq1<= 0xBF && 0x80 <=seq2 && seq2<=0xBF)
					break;
			}
			else if( (0xE1 <= seq0 && seq0 <=0xEC) || seq0==0xEE || seq0==0xEF) { // stright 3 bytes
				if(	0x80 <=seq1 && seq1<=0xBF &&
					0x80 <=seq2 && seq2<=0xBF)
					break;
			}
			else if(seq0 == 0xED) { // exclude surrogates
				if(	0x80 <=seq1 && seq1<=0x9F &&
					0x80 <=seq2 && seq2<=0xBF)
					break;
			}
			return utf::illegal;
		case 4:
			switch(seq0) {
			case 0xF0: // planes 1-3
				if(	0x90 <=seq1 && seq1<=0xBF &&
					0x80 <=seq2 && seq2<=0xBF &&
					0x80 <=seq3 && seq3<=0xBF)
					break;
				return utf::illegal;
			case 0xF1: // planes 4-15
			case 0xF2:
			case 0xF3:
				if(	0x80 <=seq1 && seq1<=0xBF &&
					0x80 <=seq2 && seq2<=0xBF &&
					0x80 <=seq3 && seq3<=0xBF)
					break;
				return utf::illegal;
			case 0xF4: // pane 16
				if(	0x80 <=seq1 && seq1<=0x8F &&
					0x80 <=seq2 && seq2<=0xBF &&
					0x80 <=seq3 && seq3<=0xBF)
					break;
				return utf::illegal;
			default:
				return utf::illegal;
			}

		}
		if(!decode)
			return 1;
		switch(len) {
		case 1:
			return seq0;
		case 2:
			return ((seq0 & 0x1F) << 6) | (seq1 & 0x3F);
		case 3:
			return ((seq0 & 0x0F) << 12) | ((seq1 & 0x3F) << 6) | (seq2 & 0x3F)  ;
		case 4:
			return ((seq0 & 0x07) << 18) | ((seq1 & 0x3F) << 12) | ((seq2 & 0x3F) << 6) | (seq3 & 0x3F) ;
		}
		return utf::illegal;
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
	struct seq {
		char c[4];
		unsigned len;
	};
	inline seq encode(uint32_t value)
	{
		seq out={ {0} };
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
		return (uint32_t(w1 & 0x3FF) << 10) | (w2 & 0x3FF) | 0x100000;
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
		seq out={ {0} };
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
