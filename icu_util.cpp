#define CPPCMS_SOURCE
#include "icu_util.h"
#ifdef HAVE_ICU

#include "locale_charset.h"
#include "utf_iterator.h"


namespace cppcms { namespace impl {

	std::string icu_to_utf8(icu::UnicodeString const &str) 
	{
		std::string tmp;
		tmp.reserve(str.length());
		uint16_t const *begin=reinterpret_cast<uint16_t const *>(str.getBuffer());
		uint16_t const *end=begin+str.length();
		while(begin<end) {
			uint32_t v=utf16::next(begin,end);
			if(v!=utf::illegal) {
				utf8::seq s=utf8::encode(v);
				tmp.append(s.c,s.len);
			}
		}
		return tmp;
	}

	icu::UnicodeString utf8_to_icu(std::string const &s)
	{
		icu::UnicodeString tmp(int32_t(s.size()),0,0);
		char const *begin=s.data();
		char const *end=begin+s.size();
		
		while(begin < end) {
			uint32_t v=utf8::next(begin,end);
			if(v!=utf::illegal)
				tmp.append((UChar32)(v));
		}
		return tmp;
	}
	
	std::string icu_to_std(icu::UnicodeString const &str,std::locale const &l)
	{
		uint16_t const *begin=reinterpret_cast<uint16_t const *>(str.getBuffer());
		uint16_t const *end=begin+str.length();
		return std::use_facet<locale::charset>(l).from_utf16(begin,end);
	}

	icu::UnicodeString std_to_icu(std::string const &str,std::locale const &l)
	{
		char const *begin=str.data();
		char const *end=str.data()+str.size();
		std::basic_string<uint16_t> tmp = std::use_facet<locale::charset>(l).to_utf16(begin,end);
		return icu::UnicodeString(reinterpret_cast<UChar const *>(tmp.data()),tmp.size());
	}

} } // cppcms::impl
