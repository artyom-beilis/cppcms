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
#ifndef CPPCMS_ENCODING_VALIDATORS_H
#define CPPCMS_ENCODING_VALIDATORS_H

#include "utf_iterator.h"

namespace cppcms { namespace encoding {

	template<typename Iterator>
	bool utf8_valid(Iterator p,Iterator e,size_t &count)
	{
		return utf8::validate(p,e,count,true);
	}

	template<typename Iterator>
	bool ascii_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7E < c)
				return false;
		}
		return true;
	}
	template<typename Iterator>
	bool iso_8859_1_2_4_5_9_10_13_14_15_16_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
		}
		return true;
	}
	template<typename Iterator>
	bool iso_8859_3_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			switch(c) {
			case 0xA5:
			case 0xAE:
			case 0xBE:
			case 0xC3:
			case 0xD0:
			case 0xE3:
			case 0xF0:
				return false;
			}
		}
		return true;
	}
	template<typename Iterator>
	bool iso_8859_6_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			if(	(0xA1<=c && c<=0xA3) ||
			 	(0xA5 <=c && c<= 0xAB) || 
				(0xAE <=c && c<= 0xBA) ||
				(0xBC <=c && c<= 0xBE) ||
				 0xC0 == c		||
				(0xDB <=c && c<= 0xDF) ||
				(0xF3 <=c && c<= 0xFF))
			{
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool iso_8859_7_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			switch(c) {
			case 0xAE:
			case 0xD2:
			case 0xFF:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool iso_8859_8_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			switch(c) {
			case 0xA1:
			case 0xFB:
			case 0xFC:
			case 0xFF:
				return false;
			}
			if(0xBF <=c && c<=0xDE)
				return false;
		}
		return true;
	}

	template<typename Iterator>
	bool iso_8859_11_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || (0x7F<=c && c<0xA0))
				return false;
			switch(c) {
			case 0xDB:
			case 0xDC:
			case 0xDD:
			case 0xDE:
			case 0xFC:
			case 0xFD:
			case 0xFE:
			case 0xFF:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1250_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x83:
			case 0x88:
			case 0x90:
			case 0x98:
				return false;
			}
		}
		return true;
	}
	template<typename Iterator>
	bool windows_1251_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c || c==152)
				return false;
		}
		return true;
	}
	template<typename Iterator>
	bool windows_1252_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x8D:
			case 0x8F:
			case 0x90:
			case 0x9D:
				return false;
			}
		}
		return true;
	}
	template<typename Iterator>
	bool windows_1253_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x88:
			case 0x8A:
			case 0x8C:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			case 0x90:
			case 0x98:
			case 0x9A:
			case 0x9C:
			case 0x9D:
			case 0x9E:
			case 0x9F:
			case 0xAA:
			case 0xD2:
			case 0xFF:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1254_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			case 0x90:
			case 0x9D:
			case 0x9E:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1255_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x8A:
			case 0x8C:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			
			case 0x90:
			case 0x9A:
			case 0x9C:
			case 0x9D:
			case 0x9E:
			case 0x9F:

			case 0xD9:
			case 0xDA:
			case 0xDB:
			case 0xDC:
			case 0xDD:
			case 0xDE:
			case 0xDF:

			case 0xCA:
			case 0xFB:
			case 0xFC:
			case 0xFF:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1256_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c)
				return false;
		}
		return true;
	}
	template<typename Iterator>
	bool windows_1257_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c)
				return false;
			switch(c) {
			case 0x81:
			case 0x83:
			case 0x88:
			case 0x8A:
			case 0x8C:
			case 0x90:
			case 0x98:
			case 0x9A:
			case 0x9C:
			case 0x9F:
			case 0xA1:
			case 0xA5:
				return false;
			}
		}
		return true;
	}

	template<typename Iterator>
	bool windows_1258_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || c==0x7F)
				return false;
			switch(c) {
			case 0x81:
			case 0x8A:
			case 0x8D:
			case 0x8E:
			case 0x8F:
			case 0x90:
			case 0x9A:
			case 0x9D:
			case 0x9E:
				return false;
			}
		}
		return true;
	}
	template<typename Iterator>
	bool koi8_valid(Iterator p,Iterator e,size_t &count)
	{
		while(p!=e) {
			count++;
			unsigned c=(unsigned char)*p++;
			if(c==0x09 || c==0xA || c==0xD)
				continue;
			if(c<0x20 || 0x7F==c)
				return false;
		}
		return true;
	}

	// ISO 


} /* validator */ } // cppcms

#endif
