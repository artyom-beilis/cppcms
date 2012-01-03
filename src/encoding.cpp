///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "encoding_validators.h"
#include "http_protocol.h"
#include <cppcms/encoding.h>
#include "utf_iterator.h"
#include <cppcms/cppcms_error.h>
#include <cppcms/localization.h>
#include <cppcms/config.h>
#include <errno.h>
#include <string>
#include <stdexcept>
#include <iostream>

namespace cppcms {
namespace encoding {

namespace impl{ 
	struct encodings_comparator {
	public:
		bool operator()(std::string const &l,std::string const &r) const
		{
			char const *lp=l.c_str();
			char const *rp=r.c_str();
			return (*this)(lp,rp);
		}

		bool operator()(std::string const &l,char const &rp) const
		{
			char const *lp=l.c_str();
			return (*this)(lp,rp);
		}

		bool operator()(char const *lp,std::string const &r) const
		{
			char const *rp=r.c_str();
			return (*this)(lp,rp);
		}

		bool operator()(char const *lp,char const *rp) const
		{
			for(;;) {
				char left = next(lp);
				char right = next(rp);
				if(left < right)
					return true;
				if(left > right)
					return false;
				if(left==right) {
					if(left == 0)
						return false;
				}
			}
		}

	private:
		static char next(char const *&p)
		{
			while(*p!=0) {
				char c = *p++;
				if('0' <= c && c<= '9')
					return c;
				if('a' <=c && c <='z')
					return c;
				else if('A' <=c && c <='Z')
					return char(c-'A'+'a');
			}
			return 0;
		}
	};

	class validators_set {
	public:

		typedef bool (*encoding_tester_type)(char const *begin,char const *end,size_t &count);
		typedef std::map<std::string,encoding_tester_type,encodings_comparator> predefined_type;
		

		encoding_tester_type get(std::string const &name) const
		{
			predefined_type::const_iterator p = predefined_.find(name);
			if(p==predefined_.end())
				return 0;
			return p->second;			
		}

		validators_set() 
		{

			encoding_tester_type iso_tester=&iso_8859_1_2_4_5_9_10_13_14_15_16_valid<char const *>;

			
			predefined_["latin1"]=iso_tester;
			
			predefined_["iso88591"]=iso_tester;
			predefined_["iso88592"]=iso_tester;
			predefined_["iso88594"]=iso_tester;
			predefined_["iso88595"]=iso_tester;
			predefined_["iso88599"]=iso_tester;
			predefined_["iso885910"]=iso_tester;
			predefined_["iso885913"]=iso_tester;
			predefined_["iso885914"]=iso_tester;
			predefined_["iso885915"]=iso_tester;
			predefined_["iso885916"]=iso_tester;

			predefined_["iso88593"]=&iso_8859_3_valid<char const *>;
			predefined_["iso88596"]=&iso_8859_6_valid<char const *>;
			predefined_["iso88597"]=&iso_8859_7_valid<char const *>;
			predefined_["iso88598"]=&iso_8859_8_valid<char const *>;
			predefined_["iso885911"]=&iso_8859_11_valid<char const *>;

			predefined_["windows1250"]=&windows_1250_valid<char const *>;
			predefined_["windows1251"]=&windows_1251_valid<char const *>;
			predefined_["windows1252"]=&windows_1252_valid<char const *>;
			predefined_["windows1253"]=&windows_1253_valid<char const *>;
			predefined_["windows1255"]=&windows_1255_valid<char const *>;
			predefined_["windows1256"]=&windows_1256_valid<char const *>;
			predefined_["windows1257"]=&windows_1257_valid<char const *>;
			predefined_["windows1258"]=&windows_1258_valid<char const *>;

			predefined_["cp1250"]=&windows_1250_valid<char const *>;
			predefined_["cp1251"]=&windows_1251_valid<char const *>;
			predefined_["cp1252"]=&windows_1252_valid<char const *>;
			predefined_["cp1253"]=&windows_1253_valid<char const *>;
			predefined_["cp1255"]=&windows_1255_valid<char const *>;
			predefined_["cp1256"]=&windows_1256_valid<char const *>;
			predefined_["cp1257"]=&windows_1257_valid<char const *>;
			predefined_["cp1258"]=&windows_1258_valid<char const *>;

			predefined_["koi8r"]=&koi8_valid<char const *>;
			predefined_["koi8u"]=&koi8_valid<char const *>;
			
			predefined_["utf8"]=&utf8_valid<char const *>;
			predefined_["usascii"]=predefined_["ascii"]=&ascii_valid<char const *>;
		}
	private:
		predefined_type predefined_;
	} all_validators;
} // impl

bool CPPCMS_API valid_utf8(char const *begin,char const *end,size_t &count)
{
	return utf8_valid(begin,end,count);
}

bool CPPCMS_API valid(char const *encoding,char const *begin,char const *end,size_t &count)
{
	return valid(std::string(encoding),begin,end,count);
}

bool CPPCMS_API valid(std::locale const &loc,char const *begin,char const *end,size_t &count)
{
	return valid(std::use_facet<locale::info>(loc).encoding(),begin,end,count);
}

bool CPPCMS_API valid(std::string const &encoding,char const *begin,char const *end,size_t &count)
{
	impl::validators_set::encoding_tester_type tester = impl::all_validators.get(encoding);
	if(tester)
		return tester(begin,end,count);
	try {
		std::string utf8_string = booster::locale::conv::between(begin,end,"UTF-8",encoding,booster::locale::conv::stop);
		return impl::all_validators.get("utf-8")(utf8_string.c_str(),utf8_string.c_str()+utf8_string.size(),count);
	}
	catch(std::runtime_error const &e) {
		return false;
	}
}

inline bool is_utf8(char const *c_encoding)
{
	impl::encodings_comparator cmp;
	return !cmp(c_encoding,"utf8") && !cmp("utf8",c_encoding);
}

std::string CPPCMS_API to_utf8(char const *c_encoding,char const *begin,char const *end)
{
	std::string result;
	if(is_utf8(c_encoding)) {
		result.assign(begin,end-begin);
		return result;
	}
	return locale::conv::to_utf<char>(begin,end,c_encoding);
}


std::string CPPCMS_API to_utf8(char const *encoding,std::string const &str)
{
	if(is_utf8(encoding))
		return str;
	return to_utf8(encoding,str.data(),str.data()+str.size());
}

std::string CPPCMS_API to_utf8(std::locale const &loc,char const *begin,char const *end)
{
	locale::info const &inf = std::use_facet<locale::info>(loc);
	if(inf.utf8())
		return std::string(begin,end-begin);
	else
		return to_utf8(inf.encoding().c_str(),begin,end);
}

std::string CPPCMS_API to_utf8(std::locale const &loc,std::string const &str)
{
	locale::info const &inf = std::use_facet<locale::info>(loc);
	if(inf.utf8())
		return str;
	else
		return to_utf8(inf.encoding().c_str(),str);
}

////////////////////
// FROM
///////////////////


std::string CPPCMS_API from_utf8(char const *c_encoding,char const *begin,char const *end)
{
	std::string result;
	if(is_utf8(c_encoding)) {
		result.assign(begin,end-begin);
		return result;
	}
	return locale::conv::from_utf<char>(begin,end,c_encoding);
}




std::string CPPCMS_API from_utf8(char const *encoding,std::string const &str)
{
	if(is_utf8(encoding))
		return str;
	return from_utf8(encoding,str.data(),str.data()+str.size());
}

std::string CPPCMS_API from_utf8(std::locale const &loc,char const *begin,char const *end)
{
	locale::info const &inf = std::use_facet<locale::info>(loc);
	if(inf.utf8())
		return std::string(begin,end-begin);
	else
		return from_utf8(inf.encoding().c_str(),begin,end);
}

std::string CPPCMS_API from_utf8(std::locale const &loc,std::string const &str)
{
	locale::info const &inf = std::use_facet<locale::info>(loc);
	if(inf.utf8())
		return str;
	else
		return from_utf8(inf.encoding().c_str(),str);
}


namespace {
	bool validate_or_filter_utf8(char const *begin,char const *end,std::string &output,char replace)
	{
		bool valid = true;
		char const *ptr = begin;
		char const *prev = ptr;
		while(ptr < end) {
			prev = ptr;
			if(utf8::next(ptr,end,true,false) == utf::illegal) {
				valid = false;
				break;
			}
		}
		if(valid)
			return true;
		output.clear();
		output.reserve(end - begin);
		output.append(begin,prev);
		ptr = prev;
		while(ptr < end) {
			prev = ptr;
			if(utf8::next(ptr,end,true,false)!=utf::illegal) {
				output.append(prev,ptr);
				continue;
			}
			ptr = prev;
			if(utf8::next(ptr,end,false,false) == utf::illegal) {
				if(replace)
					output+=replace;
				ptr = prev + 1;
			}
			else {
				if(replace)
					output+=replace;
			}
		}
		return false;
	}

	bool validate_or_filter_single_byte_charset(
			impl::validators_set::encoding_tester_type tester,
			char const *begin,
			char const *end,
			std::string &output,
			char repl)
	{
		size_t count = 0;
		if(tester(begin,end,count))
			return true;
		output.clear();
		output.reserve(end - begin);
		for(char const *p=begin;p<end;p++) {
			size_t n = 0;
			char const *ptr=p;
			if(tester(ptr,ptr+1,n))
				output+=*p;
			else if(repl)
				output+=repl;
		}
		return false;
	}
} // anonymous

bool CPPCMS_API is_ascii_compatible(std::string const &encoding)
{
	return impl::all_validators.get(encoding)!=0;
}

bool CPPCMS_API validate_or_filter(	std::string const &encoding,
					char const *begin,char const *end,
					std::string &output,
					char replace)
{
	/// UTF-8 Case
	if(is_utf8(encoding.c_str()))
		return validate_or_filter_utf8(begin,end,output,replace);

	/// 8bit case
	impl::validators_set::encoding_tester_type tester = impl::all_validators.get(encoding);
	if(tester)
		return validate_or_filter_single_byte_charset(tester,begin,end,output,replace);

	size_t tmp_count = 0;
	/// Most common case
	if(valid(encoding,begin,end,tmp_count))
		return true;

	std::string utf8_string = booster::locale::conv::between(
			begin,end,
			"UTF-8",encoding,
			booster::locale::conv::skip);

	std::string tmp_out;

	if(validate_or_filter_utf8(utf8_string.c_str(),utf8_string.c_str() + utf8_string.size(),tmp_out,0)) {
		tmp_out.swap(utf8_string);
	}

	output = booster::locale::conv::between(
			tmp_out.c_str(),
			tmp_out.c_str()+tmp_out.size(),
			encoding,
			"UTF-8",
			booster::locale::conv::skip);
	return false;
}


} } // cppcms::encoding
