#ifndef CPPCMS_LOCALE_ICU_H
#define CPPCMS_LOCALE_ICU_H

#include "defs.h"
#include <locale>

namespace cppcms {
namespace locale {

#ifdef HAVE_ICU
	typedef icu::UnicodeString unicode_string;
#else
	typedef std::basic_string<wchar_t> unicode_string;
#endif


	class CPPCMS_API convert : public std::locale::facet {
	public:
		static std::locale::id id;

		convert(std::locale const &source,size_t refs=0);
		virtual ~convert();

		template<typename Char>
		std::basic_string<Char> to_upper(std::basic_string<Char> const &str) const
		{
			to_std<Char>(to_uni(str).do_to_upper(get()));
		}

		template<typename Char>
		std::basic_string<Char> to_lower(std::basic_string<Char> const &str) const
		{
			to_std<Char>(to_uni(str).do_to_lower(get()));
		}

		template<typename Char>
		std::basic_string<Char> to_title(std::basic_string<Char> const &str) const
		{
			to_std<Char>(to_uni(str).do_to_title(get()));
		}

		enum {	norm_default,
			norm_nfc,
			norm_nfd,
			norm_nfkc,
			norm_nfkd
		} norm_type;

		template<typename Char>
		std::basic_string<Char> to_normal(std::basic_string<Char> const &str,norm_type how=normalize_default) const
		{
			to_std<Char>(do_to_normal(to_icu(str),how));
		}

		template<Char>
		std::basic_string<Char> to_std(unicode_string const &str);

		template<Char>
		unicode_string to_uni(std::basic_string<Char> const &str);

	private:
		
		unicode_string do_to_upper(unicode_string const &s) const;
		unicode_string do_to_lower(unicode_string const &s) const;
		unicode_string do_to_title(unicode_string const &s) const;
		unicode_string do_to_normal(unicode_string const &s) const;

		struct data;
		util::hold_ptr<data> d;
			
	};

	template<>
	std::basic_string<char> CPPCMS_API unicode::to_std(unicode_string const &str);

	template<>
	unicode_string CPPCMS_API unicode::to_uni(std::basic_string<char> const &str);

	template<>
	std::basic_string<wchar_t> CPPCMS_API unicode::to_std(unicode_string const &str);

	template<>
	unicode_string cppcms_api unicode::to_uni(std::basic_string<wchar_t> const &str);

	template<>
	std::basic_string<uint16_t> cppcms_api unicode::to_std(unicode_string const &str);

	template<>
	unicode_string cppcms_api unicode::to_uni(std::basic_string<uint16_t> const &str);

	template<>
	std::basic_string<uint32_t> cppcms_api unicode::to_std(unicode_string const &str);

	template<>
	unicode_string cppcms_api unicode::to_uni(std::basic_string<uint32_t> const &str);

	#ifdef HAVE_CPP0X_UXSTRING
	template<>
	std::basic_string<char16_t> CPPCMS_API unicode::to_std(unicode_string const &str);

	template<>
	unicode_string CPPCMS_API unicode::to_uni(std::basic_string<char16_t> const &str);

	template<>
	std::basic_string<char32_t> CPPCMS_API unicode::to_std(unicode_string const &str);

	template<>
	unicode_string CPPCMS_API unicode::to_uni(std::basic_string<char32_t> const &str);

	#endif

} // locale
} // cppcms


#endif
