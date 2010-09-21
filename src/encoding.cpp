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
#include "encoding_validators.h"
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
	struct validators_set {
		
		typedef bool (*encoding_tester_type)(char const *begin,char const *end,size_t &count);

		encoding_tester_type get(char const *str) const
		{
			std::string name=str;
			for(unsigned i=0;i<name.size();i++)
				if('A' <= name[i]  && name[i] <= 'Z')
					name[i]-=('A'-'a');
			std::map<std::string,encoding_tester_type>::const_iterator p;
			p=predefined_.find(name);
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

			predefined_["8859_1"]=iso_tester;
			predefined_["8859_2"]=iso_tester;
			predefined_["8859_4"]=iso_tester;
			predefined_["8859_5"]=iso_tester;
			predefined_["8859_9"]=iso_tester;
			predefined_["8859_10"]=iso_tester;
			predefined_["8859_13"]=iso_tester;
			predefined_["8859_14"]=iso_tester;
			predefined_["8859_15"]=iso_tester;
			predefined_["8859_16"]=iso_tester;

			predefined_["iso8859-1"]=iso_tester;
			predefined_["iso8859-2"]=iso_tester;
			predefined_["iso8859-4"]=iso_tester;
			predefined_["iso8859-5"]=iso_tester;
			predefined_["iso8859-9"]=iso_tester;
			predefined_["iso8859-10"]=iso_tester;
			predefined_["iso8859-13"]=iso_tester;
			predefined_["iso8859-14"]=iso_tester;
			predefined_["iso8859-15"]=iso_tester;
			predefined_["iso8859-16"]=iso_tester;
			
			predefined_["iso_8859-1"]=iso_tester;
			predefined_["iso_8859-2"]=iso_tester;
			predefined_["iso_8859-4"]=iso_tester;
			predefined_["iso_8859-5"]=iso_tester;
			predefined_["iso_8859-9"]=iso_tester;
			predefined_["iso_8859-10"]=iso_tester;
			predefined_["iso_8859-13"]=iso_tester;
			predefined_["iso_8859-14"]=iso_tester;
			predefined_["iso_8859-15"]=iso_tester;
			predefined_["iso_8859-16"]=iso_tester;

			predefined_["iso-8859-1"]=iso_tester;
			predefined_["iso-8859-2"]=iso_tester;
			predefined_["iso-8859-4"]=iso_tester;
			predefined_["iso-8859-5"]=iso_tester;
			predefined_["iso-8859-9"]=iso_tester;
			predefined_["iso-8859-10"]=iso_tester;
			predefined_["iso-8859-13"]=iso_tester;
			predefined_["iso-8859-14"]=iso_tester;
			predefined_["iso-8859-15"]=iso_tester;
			predefined_["iso-8859-16"]=iso_tester;

			predefined_["iso88593"]=&iso_8859_3_valid<char const *>;
			predefined_["iso88596"]=&iso_8859_6_valid<char const *>;
			predefined_["iso88597"]=&iso_8859_7_valid<char const *>;
			predefined_["iso88598"]=&iso_8859_8_valid<char const *>;
			predefined_["iso885911"]=&iso_8859_11_valid<char const *>;

			predefined_["iso8859-3"]=&iso_8859_3_valid<char const *>;
			predefined_["iso8859-6"]=&iso_8859_6_valid<char const *>;
			predefined_["iso8859-7"]=&iso_8859_7_valid<char const *>;
			predefined_["iso8859-8"]=&iso_8859_8_valid<char const *>;
			predefined_["iso8859-11"]=&iso_8859_11_valid<char const *>;
			
			predefined_["8859_3"]=&iso_8859_3_valid<char const *>;
			predefined_["8859_6"]=&iso_8859_6_valid<char const *>;
			predefined_["8859_7"]=&iso_8859_7_valid<char const *>;
			predefined_["8859_8"]=&iso_8859_8_valid<char const *>;
			predefined_["8859_11"]=&iso_8859_11_valid<char const *>;

			predefined_["iso_8859-3"]=&iso_8859_3_valid<char const *>;
			predefined_["iso_8859-6"]=&iso_8859_6_valid<char const *>;
			predefined_["iso_8859-7"]=&iso_8859_7_valid<char const *>;
			predefined_["iso_8859-8"]=&iso_8859_8_valid<char const *>;
			predefined_["iso_8859-11"]=&iso_8859_11_valid<char const *>;
		
			predefined_["iso-8859-3"]=&iso_8859_3_valid<char const *>;
			predefined_["iso-8859-6"]=&iso_8859_6_valid<char const *>;
			predefined_["iso-8859-7"]=&iso_8859_7_valid<char const *>;
			predefined_["iso-8859-8"]=&iso_8859_8_valid<char const *>;
			predefined_["iso-8859-11"]=&iso_8859_11_valid<char const *>;

			predefined_["windows-1250"]=&windows_1250_valid<char const *>;
			predefined_["windows-1251"]=&windows_1251_valid<char const *>;
			predefined_["windows-1252"]=&windows_1252_valid<char const *>;
			predefined_["windows-1253"]=&windows_1253_valid<char const *>;
			predefined_["windows-1255"]=&windows_1255_valid<char const *>;
			predefined_["windows-1256"]=&windows_1256_valid<char const *>;
			predefined_["windows-1257"]=&windows_1257_valid<char const *>;
			predefined_["windows-1258"]=&windows_1258_valid<char const *>;

			predefined_["cp1250"]=&windows_1250_valid<char const *>;
			predefined_["cp1251"]=&windows_1251_valid<char const *>;
			predefined_["cp1252"]=&windows_1252_valid<char const *>;
			predefined_["cp1253"]=&windows_1253_valid<char const *>;
			predefined_["cp1255"]=&windows_1255_valid<char const *>;
			predefined_["cp1256"]=&windows_1256_valid<char const *>;
			predefined_["cp1257"]=&windows_1257_valid<char const *>;
			predefined_["cp1258"]=&windows_1258_valid<char const *>;

			predefined_["1250"]=&windows_1250_valid<char const *>;
			predefined_["1251"]=&windows_1251_valid<char const *>;
			predefined_["1252"]=&windows_1252_valid<char const *>;
			predefined_["1253"]=&windows_1253_valid<char const *>;
			predefined_["1255"]=&windows_1255_valid<char const *>;
			predefined_["1256"]=&windows_1256_valid<char const *>;
			predefined_["1257"]=&windows_1257_valid<char const *>;
			predefined_["1258"]=&windows_1258_valid<char const *>;

			predefined_["koi8r"]=predefined_["koi8-r"]=&koi8_valid<char const *>;
			predefined_["koi8u"]=predefined_["koi8-u"]=&koi8_valid<char const *>;
			
			predefined_["utf8"]=predefined_["utf-8"]=&utf8_valid<char const *>;
			predefined_["us-ascii"]=predefined_["ascii"]=&ascii_valid<char const *>;
		}
	private:
		std::map<std::string,encoding_tester_type> predefined_;
	} all_validators;
} // impl

bool CPPCMS_API valid_utf8(char const *begin,char const *end,size_t &count)
{
	return utf8_valid(begin,end,count);
}

bool CPPCMS_API valid(std::string const &encoding,char const *begin,char const *end,size_t &count)
{
	return valid(encoding.c_str(),begin,end,count);
}

bool CPPCMS_API valid(std::locale const &loc,char const *begin,char const *end,size_t &count)
{
	return valid(std::use_facet<locale::info>(loc).encoding().c_str(),begin,end,count);
}

bool CPPCMS_API valid(char const *encoding,char const *begin,char const *end,size_t &count)
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
	return 	strcmp(c_encoding,"UTF8")==0 
		|| strcmp(c_encoding,"UTF-8")==0
		|| strcmp(c_encoding,"utf8")==0
		|| strcmp(c_encoding,"utf-8")==0;
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
	
} } // cppcms::encoding
