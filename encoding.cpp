#include "encoding_validators.h"
#include "encoding.h"
#include <iconv.h>
#include <errno.h>
#include <string>
#include <stdexcept>

namespace cppcms { namespace encoding {

	class iconv_validator {
	private:
		iconv_t descriptor_;

		iconv_validator(iconv_validator const &other);
		iconv_validator const &operator=(iconv_validator const &);

	public:
		iconv_validator(std::string const &charset) :
			descriptor_((iconv_t)(-1))
		{
			char const *le="UTF-32LE";
			char const *be="UTF-32BE";
			uint16_t v=0x0a0b;
			char const *utf=*(char*)&v== 0x0a ? be : le;

			descriptor_=iconv_open(utf,charset.c_str());
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
		predefined_["utf-8"]=&utf8_valid<char const *>;

		encoding_tester_type iso_tester=&iso_8859_1_2_4_5_9_10_13_14_15_16_valid<char const *>;

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

		predefined_["koi8-r"]=&koi8_valid<char const *>;
		predefined_["koi8-u"]=&koi8_valid<char const *>;
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

} } // cppcms::encoding
