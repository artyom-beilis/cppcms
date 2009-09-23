#ifndef CPPCMS_LOCALE_CHARSET_H
#define CPPCMS_LOCALE_CHARSET_H

#include "defs.h"
#include "hold_ptr.h"
#include "intrusive_ptr.h"
#include "config.h"

#include <locale>
#include <string>

#ifndef HAVE_STD_WSTRING
namespace std {
	typedef basic_string<wchar_t> wstring;
}
#endif


namespace cppcms { 
	namespace encoding {
		class validators_set;
	}
	
namespace locale {

	class CPPCMS_API charset : public std::locale::facet {
	public:
		static std::locale::id id;
		charset(std::size_t refs=0);
		charset(std::string charset,std::size_t refs=0);
		charset(std::string charset,intrusive_ptr<encoding::validators_set> p,std::size_t refs=0);
		~charset();
		
		bool validate(char const *begin,char const *end,size_t &count) const
		{
			return do_validate(begin,end,count);
		}
		bool validate(std::string const &s,size_t &count) const
		{
			return do_validate(s.data(),s.data()+s.size(),count);
		}

		std::string to_utf8(std::string const &v) const
		{
			return do_to_utf8(v);
		}

		std::string from_utf8(std::string const &v) const
		{
			return do_from_utf8(v);
		}

		std::basic_string<uint16_t> to_utf16(char const *begin,char const *end) const
		{
			return do_to_utf16(begin,end);
		}

		std::string from_utf16(uint16_t const *begin,uint16_t const *end) const
		{
			return do_from_utf16(begin,end);
		}

		std::basic_string<uint32_t> to_utf32(char const *begin,char const *end) const
		{
			return do_to_utf32(begin,end);
		}

		std::string from_utf32(uint32_t const *begin,uint32_t const *end) const
		{
			return do_from_utf32(begin,end);
		}

		std::wstring to_wstring(std::string const &v) const
		{
			#if SIZEOF_WCHAR_T==2
			std::basic_string<uint16_t> tmp=do_to_utf16(v.data(),v.data()+v.size());
			#else
			std::basic_string<uint32_t> tmp=do_to_utf32(v.data(),v.data()+v.size());
			#endif
			return std::wstring(tmp.begin(),tmp.end());
		}
		std::string from_wstring(std::wstring const &v) const
		{
			#if SIZEOF_WCHAR_T==2
			uint16_t const *begin=reinterpret_cast<uint16_t const *>(v.data());
			uint16_t const *end=begin+v.size();
			return do_from_utf16(begin,end);
			#else
			uint32_t const *begin=reinterpret_cast<uint32_t const *>(v.data());
			uint32_t const *end=begin+v.size();
			return do_from_utf32(begin,end);
			#endif
		}

#ifdef HAVE_CPP0X_UXSTRING
		std::u16string to_u16string(std::string const &v) const
		{
			std::basic_string<uint16_t> tmp=do_to_utf16(v.data(),v.data()+v.size());
			return std::u16string(tmp.begin(),tmp.end());
		}
		std::string from_u16string(std::u16string const &v) const
		{
			uint16_t const *begin=reinterpret_cast<uint16_t const *>(v.data());
			uint16_t const *end=begin+v.size();
			return do_from_utf16(begin,end);
		}
		std::u32string to_u32string(std::string const &v) const
		{
			std::basic_string<uint32_t> tmp=do_to_utf32(v.data(),v.data()+v.size());
			return std::u32string(tmp.begin(),tmp.end());
		}
		std::string from_u32string(std::u32string const &v) const
		{
			uint32_t const *begin=reinterpret_cast<uint32_t const *>(v.data());
			uint32_t const *end=begin+v.size();
			return do_from_utf32(begin,end);
		}
#endif

	private:
		virtual bool do_validate(char const *begin,char const *end,size_t &count) const;

		virtual std::string do_to_utf8(std::string const &v) const;
		virtual std::string do_from_utf8(std::string const &v) const;

		virtual std::basic_string<uint16_t> do_to_utf16(char const *begin,char const *end) const;
		virtual std::string do_from_utf16(uint16_t const *begin,uint16_t const *end) const;

		virtual std::basic_string<uint32_t> do_to_utf32(char const *begin,char const *end) const;
		virtual std::string do_from_utf32(uint32_t const *begin,uint32_t const *end) const;
		
		struct data;
		util::hold_ptr<data> d;
		std::string name_;
		bool is_utf8_;
		intrusive_ptr<encoding::validators_set> validators_;

	};


} } // cppcms::locale

#endif
