#define CPPCMS_SOURCE
#include "encoding_validators.h"
#include "encoding.h"
#include "utf_iterator.h"
#include "cppcms_error.h"
#include "localization.h"
#include "config.h"
#include <errno.h>
#include <string>
#include <stdexcept>
#include <iostream>

#ifdef HAVE_ICONV
#include <iconv.h>
#else
#include <unicode/ucnv.h> // For Win32
#endif



namespace cppcms {
namespace encoding {

namespace impl{ 
#ifdef HAVE_ICONV
	typedef size_t (*posix_iconv_type)(iconv_t cd,char **, size_t *t,char **,size_t *);
	typedef size_t (*gnu_iconv_type)(iconv_t cd,char const **, size_t *t,char **,size_t *);

	size_t do_iconv(posix_iconv_type cv,iconv_t cd,char const **inbuf, size_t *inbytesleft,char **outbuf,size_t *outbytesleft)
	{
		return cv(cd,const_cast<char **>(inbuf),inbytesleft,outbuf,outbytesleft);
	}
	size_t do_iconv(gnu_iconv_type cv,iconv_t cd,char const **inbuf, size_t *inbytesleft,char **outbuf,size_t *outbytesleft)
	{
		return cv(cd,inbuf,inbytesleft,outbuf,outbytesleft);
	}

	char const *native_utf32_encoding()
	{
		char const *le="UTF-32LE";
		char const *be="UTF-32BE";
		uint16_t v=0x0a0b;
		char const *utf=*(char*)&v== 0x0a ? be : le;
		return utf;
	}
	
	char const *native_utf16_encoding()
	{
		char const *le="UTF-16LE";
		char const *be="UTF-16BE";
		uint16_t v=0x0a0b;
		char const *utf=*(char*)&v== 0x0a ? be : le;
		return utf;
	}

	char const *native_wchar_encoding()
	{
		if(sizeof(wchar_t)==4)
			return native_utf32_encoding();
		if(sizeof(wchar_t)==2)
			return native_utf16_encoding();
		throw std::runtime_error("wchar_t does not support unicode!");
	}
	
	class iconv_validator {
	public:
		iconv_validator(std::string const &charset) :
			descriptor_((iconv_t)(-1))
		{
			descriptor_=iconv_open(native_utf32_encoding(),charset.c_str());
			if(descriptor_==(iconv_t)(-1)) {
				throw std::runtime_error("Failed to load iconv tables for:" + charset);
			}
		}

		~iconv_validator()
		{
			if(descriptor_!=(iconv_t)(-1))
				iconv_close(descriptor_);
		}

		bool valid(char const *begin,char const *end,size_t &count)
		{
			iconv(descriptor_,0,0,0,0); // reset
			uint32_t buffer[64];
			size_t input=end-begin;
			while(begin!=end) {
				uint32_t *output=buffer;
				size_t outsize=sizeof(buffer);

				size_t res=do_iconv(::iconv,descriptor_,&begin,&input,(char**)&output,&outsize);

				if(res==(size_t)(-1) && errno==E2BIG)  {
					if(!check_symbols(buffer,output,count))
						return false;
					continue;
				}
				if(res!=(size_t)(-1) && input==0)
					return check_symbols(buffer,output,count);
				return false;
			}
			return true;
		}
	private:
		iconv_t descriptor_;

		iconv_validator(iconv_validator const &other);
		iconv_validator const &operator=(iconv_validator const &);

		bool check_symbols(uint32_t const *begin,uint32_t const *end,size_t &count)
		{
			while(begin!=end) {
				uint32_t c=*begin++;
				count++;
				if(c==0x09 || c==0xA || c==0xD)
					continue;
				if(c<0x20 || (0x7F<=c && c<=0x9F))
					return false;
			}
			return true;
		}
	};


	typedef iconv_validator validator;
#else // NO HAVE_ICONV
	class uconv_validator {
	public:
		uconv_validator(std::string const &charset) :
			uconv_(0)
		{
			UErrorCode err=U_ZERO_ERROR;
			uconv_=ucnv_open(charset.c_str(),&err);
			if(!uconv_)
				throw cppcms_error("Invalid encoding:" + charset + u_errorName(err));
			err=U_ZERO_ERROR;
			ucnv_setToUCallBack(uconv_,UCNV_TO_U_CALLBACK_STOP,0,0,0,&err);
			if(U_FAILURE(err)) {
				ucnv_close(uconv_);
				throw cppcms_error("Invalid encoding:" + charset + u_errorName(err));
			}
		}

		~uconv_validator()
		{
			if(uconv_) ucnv_close(uconv_);
		}

		bool valid(char const *begin,char const *end,size_t &count)
		{
			UChar buffer[64];
			UChar *ubegin=buffer;
			UChar *uend=ubegin+64;
			count = 0;
			while(begin!=end) {
				UErrorCode err=U_ZERO_ERROR;
				ucnv_toUnicode(uconv_,&ubegin,uend,&begin,end,0,1,&err);
				if(err==U_BUFFER_OVERFLOW_ERROR) {
					if(!check_symbols(buffer,ubegin,count))
						return false;
				}
				else if(U_FAILURE(err))
					return false;
			}
			return true;
		}

	private:
		UConverter *uconv_;

		uconv_validator(uconv_validator const &other);
		void operator=(uconv_validator const &);
		
		bool check_symbols(UChar const *begin,UChar const *end,size_t &count)
		{
			while(begin!=end) {
				UChar c=*begin++;
				if(!U_IS_SURROGATE(c) || U_IS_SURROGATE_LEAD(c))
					count++;
				if(c==0x09 || c==0xA || c==0xD)
					continue;
				if(c<0x20 || (0x7F<=c && c<=0x9F))
					return false;
			}
			return true;
		}
	};

	typedef uconv_validator validator;
#endif
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
		impl::validator vtester(encoding);
		return vtester.valid(begin,end,count);
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


#ifdef HAVE_ICONV
namespace impl {
	std::string iconv_convert_to(char const *to,char const *from,char const *begin,char const *end)
	{
		iconv_t d=iconv_open(to,from);
		if(d==(iconv_t)(-1)) 
			throw cppcms_error("Unsupported encoding "+std::string(to));
		
		std::string result;
		try {
			char buffer[256];
			size_t input=end-begin;
			while(begin!=end) {
				char *output=buffer;
				size_t outsize=sizeof(buffer);

				size_t res=do_iconv(::iconv,d,&begin,&input,(char**)&output,&outsize);

				if(res==(size_t)(-1) && errno==E2BIG)  {
					result.append(buffer,output);
					continue;
				}
				if(res!=(size_t)(-1) && input==0) {
					result.append(buffer,output);
					break;
				}
				break;
			}
			
		}
		catch(...) {
			iconv_close(d);
		}
		iconv_close(d);
		return result;
	}
} // impl
#endif


std::string CPPCMS_API to_utf8(char const *c_encoding,char const *begin,char const *end)
{
	std::string result;
	if(is_utf8(c_encoding)) {
		result.assign(begin,end-begin);
		return result;
	}
#ifdef HAVE_ICONV
	return impl::iconv_convert_to("UTF-8",c_encoding,begin,end);
#else // USE ICU
	locale::details::converter cvt(c_encoding);
	result.reserve(end-begin);

	std::vector<char> buf(cvt.max_len() * 64);

	while(begin<end) {
		uint32_t u=utf8::next(begin,end,false,true);
		if(u > 0x10FFFF) // error
			return result;
		char *tbegin=&buf[0];
		char *tend=tbegin+buf.size();
		uint32_t n = cvt.from_unicode(u ,tbegin,tend);
		if(n != locale::details::converter::illegal && n!= locale::details::converter::incomplete)
			result.append(tbegin,tbegin+n);
		else
			return result;
	}
	return result;
#endif
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
#ifdef HAVE_ICONV
	return impl::iconv_convert_to(c_encoding,"UTF-8",begin,end);
#else // USE ICU
	locale::details::converter cvt(c_encoding);
	result.reserve(end-begin);

	while(begin<end) {
		uint32_t u=cvt.to_unicode(begin,end);
		if(u > 0x10FFFF) // error
			return result;
		utf8::seq s=utf8::encode(u);
		result.append(s.c,s.len);
	}
	return result;
#endif
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
