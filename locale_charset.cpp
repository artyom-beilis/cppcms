#define CPPCMS_SOURCE
#include "locale_charset.h"
#include "encoding.h"
#include "utf_iterator.h"
#include "cppcms_error.h"
#include <iostream>

namespace cppcms {
namespace locale {

struct charset::data {};

std::locale::id charset::id;

charset::charset(std::size_t refs) :
	std::locale::facet(refs),
	is_utf8_(true),
	validators_(0)
{
}


charset::charset(std::string charset,std::size_t refs) :
	std::locale::facet(refs),
	name_(charset),
	validators_(0)
{
	is_utf8_ = charset=="utf8" || charset=="UTF8" || charset=="utf-8" || charset=="UTF-8";
}


charset::charset(std::string charset,intrusive_ptr<encoding::validators_set> p,std::size_t refs) :
	std::locale::facet(refs),
	name_(charset),
	validators_(p)
{
	is_utf8_ = charset=="utf8" || charset=="UTF8" || charset=="utf-8" || charset=="UTF-8";
}

charset::~charset()
{
}


bool charset::do_validate(char const *begin,char const *end,size_t &count) const
{
	if(!validators_)
		return true;
	encoding::validator v=(*validators_)[name_];
	return v.valid(begin,end,count);
}
std::string charset::do_to_utf8(std::string const &v) const
{
	if(is_utf8_)
		return v;
	encoding::converter conv(name_);
	return conv.to_utf8(v.data(),v.data()+v.size());
		
}
std::string charset::do_from_utf8(std::string const &v) const
{
	if(is_utf8_)
		return v;
	encoding::converter conv(name_);
	return conv.from_utf8(v.data(),v.data()+v.size());
}


std::basic_string<uint16_t> charset::do_to_utf16(char const *begin,char const *end) const
{
	if(is_utf8_) {
		std::basic_string<uint16_t> result;
		result.reserve(end-begin);
		while(begin < end) {
			uint32_t code_point=utf8::next(begin,end,false,true);
			if(code_point==utf::illegal)
				throw cppcms_error("Invalid utf8");
			utf16::seq s=utf16::encode(code_point);
			result.append(s.c,s.len);
		}
		return result;
	}
	encoding::converter conv(name_);
	return conv.to_utf16(begin,end);
}

std::basic_string<uint32_t> charset::do_to_utf32(char const *begin,char const *end) const
{
	if(is_utf8_) {
		std::basic_string<uint32_t> result;
		result.reserve(end-begin);
		while(begin < end) {
			uint32_t code_point=utf8::next(begin,end,false,true);
			if(code_point==utf::illegal)
				throw cppcms_error("Invalid utf8");
			result+=code_point;
		}
		return result;
	}

	encoding::converter conv(name_);
	return conv.to_utf32(begin,end);
}


std::string charset::do_from_utf16(uint16_t const *begin,uint16_t const *end) const
{
	if(is_utf8_) {
		std::string result;
		result.reserve(end-begin);
		while(begin < end) {
			uint32_t code_point=utf16::next(begin,end);
			if(code_point==utf::illegal)
				throw cppcms_error("Invalid utf16");
			utf8::seq s=utf8::encode(code_point);
			result.append(s.c,s.len);
		}
		return result;
	}
	encoding::converter conv(name_);
	return conv.from_utf16(begin,end);
}


std::string charset::do_from_utf32(uint32_t const *begin,uint32_t const *end) const
{
	if(is_utf8_) {
		std::string result;
		result.reserve(end-begin);
		while(begin < end) {
			uint32_t code_point=*begin++;
			if(!utf::valid(code_point))
				throw cppcms_error("Invalid utf32");
			utf8::seq s=utf8::encode(code_point);
			result.append(s.c,s.len);
		}
		return result;
	}
	encoding::converter conv(name_);
	return conv.from_utf32(begin,end);
}

} // locale
} // cppcms
