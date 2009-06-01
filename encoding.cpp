#include "encoding_validators.h"
#include "encoding.h"
#include <iconv.h>
#include <errno.h>
#include <string>
#include <stdexcept>
#include <iostream>

namespace cppcms { namespace encoding {

	namespace {
		char const *native_unicode_encoding()
		{
			char const *le="UTF-32LE";
			char const *be="UTF-32BE";
			uint16_t v=0x0a0b;
			char const *utf=*(char*)&v== 0x0a ? be : le;
			return utf;
		}
		char const *native_wchar_encoding()
		{
			if(sizeof(wchar_t)==4)
				return native_unicode_encoding();
			if(sizeof(wchar_t)!=2)
				throw std::runtime_error("wchar_t does not support unicode!");
			char const *le="UTF-16LE";
			char const *be="UTF-16BE";
			uint16_t v=0x0a0b;
			char const *utf=*(char*)&v== 0x0a ? be : le;
			return utf;
		}
	}
	
	class iconv_validator {
	private:
		iconv_t descriptor_;

		iconv_validator(iconv_validator const &other);
		iconv_validator const &operator=(iconv_validator const &);

	public:
		iconv_validator(std::string const &charset) :
			descriptor_((iconv_t)(-1))
		{
			descriptor_=iconv_open(native_unicode_encoding(),charset.c_str());
			if(descriptor_==(iconv_t)(-1)) {
				throw std::runtime_error("Failed to load iconv tables for:" + charset);
			}
		}

		~iconv_validator()
		{
			if(descriptor_!=(iconv_t)(-1))
				iconv_close(descriptor_);
		}
		bool check_symbols(uint32_t const *begin,uint32_t const *end)
		{
			while(begin!=end) {
				uint32_t c=*begin++;
				if(c==0x09 || c==0xA || c==0xD)
					continue;
				if(c<0x20 || (0x7F<=c && c<=0x9F))
					return false;
			}
			return true;
		}

		bool valid(char const *begin,char const *end)
		{
			iconv(descriptor_,0,0,0,0); // reset
			uint32_t buffer[64];
			size_t input=end-begin;
			while(begin!=end) {
				uint32_t *output=buffer;
				size_t outsize=sizeof(buffer);

				size_t res=iconv(descriptor_,const_cast<char**>(&begin),&input,(char**)&output,&outsize);

				if(res==(size_t)(-1) && errno==E2BIG)  {
					if(!check_symbols(buffer,output))
						return false;
					continue;
				}
				if(res!=(size_t)(-1) && input==0)
					return check_symbols(buffer,output);
				return false;
			}
			return true;
		}

	};


	validator::validator(encoding_tester_type enc) :
		tester_(enc),
		iconv_(0)
	{
	}
	validator::validator(std::string s):
		charset_(s),
		tester_(0),
		iconv_(new iconv_validator(s))
	{
	}
	validator::validator(validator const &other) :
		charset_(other.charset_),
		tester_(other.tester_),
		iconv_( other.iconv_ ? (new iconv_validator(charset_)) : 0)
	{
	}
	validator const &validator::operator=(validator const &other)
	{
		if(&other!=this) {
			if(iconv_) delete iconv_;
			charset_=other.charset_;
			tester_=other.tester_;
			if(other.iconv_)
				iconv_=new iconv_validator(charset_);
		}
		return *this;
	}
	validator::~validator()
	{
		if(iconv_)
			delete iconv_;
	}

	bool validator::valid(std::string const &s)
	{
		return valid(s.data(),s.data()+s.size());
	}
	bool validator::valid(char const *begin,char const *end)
	{
		if(tester_)
			return tester_(begin,end);
		return iconv_->valid(begin,end);
	}

	validators_set::validators_set() 
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

		predefined_["cp1250"]=predefined_["windows-1250"]=&windows_1250_valid<char const *>;
		predefined_["cp1251"]=predefined_["windows-1251"]=&windows_1251_valid<char const *>;
		predefined_["cp1252"]=predefined_["windows-1252"]=&windows_1252_valid<char const *>;
		predefined_["cp1253"]=predefined_["windows-1253"]=&windows_1253_valid<char const *>;
		predefined_["cp1255"]=predefined_["windows-1255"]=&windows_1255_valid<char const *>;
		predefined_["cp1256"]=predefined_["windows-1256"]=&windows_1256_valid<char const *>;
		predefined_["cp1257"]=predefined_["windows-1257"]=&windows_1257_valid<char const *>;
		predefined_["cp1258"]=predefined_["windows-1258"]=&windows_1258_valid<char const *>;

		predefined_["koi8"]=predefined_["koi8r"]=predefined_["koi8-r"]=&koi8_valid<char const *>;
		predefined_["koi8u"]=predefined_["koi8-u"]=&koi8_valid<char const *>;
		
		predefined_["utf8"]=predefined_["utf-8"]=&utf8_valid<char const *>;
	}

	validator validators_set::operator[](std::string s) const
	{
		for(unsigned i=0;i<s.size();i++) {
			if('A'<=s[i] && s[i]<='Z')
				s[i]=s[i]-'A'+'a';
		}
		std::map<std::string,encoding_tester_type>::const_iterator p;
		if((p=predefined_.find(s))!=predefined_.end()) {
			return validator(p->second);
		}
		return validator(s);
	}

	std::string to_string(std::wstring const &s)
	{
		std::string result;
		result.reserve(s.size());
		if(sizeof(wchar_t)==2) {
			std::wstring::const_iterator p=s.begin(),e=s.end();
			while(p!=e) {
				uint32_t point;
				if(sizeof(wchar_t)==2) {
					point=utf16::next(p,e);
					if(point==utf::illegal)
						break;
				}
				else {
					point=*p++;
					if(!utf::valid(point))
						break;
				}
				utf8::seq s=utf8::encode(point);
				result+=s.c[0];
				result.append(s.c+1);
			}
		}
		return result;
	}
	std::string to_string(std::wstring const &s,std::locale const &locale)
	{
		throw std::runtime_error("Unsuppoteed");
	}
	std::string to_string(std::wstring const &s,std::string const &encoding)
	{
		throw std::runtime_error("Unsuppoteed");
	}

	std::wstring to_wstring(std::string const &s)
	{
		std::wstring result;
		result.reserve(s.size());
		std::string::const_iterator p=s.begin(),e=s.end();
		while(p!=e) {
			std::cerr<<p-s.begin()<<std::endl;
			uint32_t point=utf8::next(p,e,false,true);
			if(point==utf::illegal)
				break;
			if(sizeof(wchar_t)==2) {
				utf16::seq s=utf16::encode(point);
				result+=wchar_t(s.c[0]);
				if(s.c[1]) result+=wchar_t(s.c[1]);
			}
			else {
				result+=wchar_t(point);
			}
		}
		return result;
	}
	std::wstring to_wstring(std::string const &s,std::locale const &locale)
	{
		throw std::runtime_error("Unsuppoteed");
	}
	std::wstring to_wstring(std::string const &s,std::string const &encoding)
	{
		throw std::runtime_error("Unsuppoteed");
	}

	
	
	
} } // cppcms::encoding
