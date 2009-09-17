#ifndef CPPCMS_LOCALE_ICU_H
#define CPPCMS_LOCALE_ICU_H

#include "defs.h"
#include "config.h"
#include "hold_ptr.h"
#include <locale>


namespace cppcms {
namespace locale {

	class convert_impl;

	///
	/// \brief This class provides basic unicode aware string manipulation utilities
	///
	/// If CppCMS is compiled with ICU support it performs these operations using ICU
	/// library, otherwise it uses std::locale facets for these operations, so some
	/// functionality may have lower quality or not supported at all
	///

	class CPPCMS_API convert : public std::locale::facet {
	public:
		static std::locale::id id;

		typedef enum {	norm_default,	/// < Default Unicode normalization as provided by ICU
				norm_nfc,	/// < NFC Unicode normalization
				norm_nfd,	/// < NFD Unicode normalization
				norm_nfkc,	/// < NFKC Unicode normalization
				norm_nfkd	/// < NFKD Unicode normalization
		} norm_type;
		

		///
		/// Create convert facet.
		///
		/// \a charset, \a icu_locale and \a info facets should be assigned to \a source
		/// 

		convert(std::locale const &source,size_t refs=0);
		virtual ~convert();

		///
		/// Convert string in current locale representation (utf8 or other encoding)
		/// to upper case.
		///
		
		std::string to_upper(std::string const &str) const;
		///
		/// Convert string in current locale representation (utf8 or other encoding)
		/// to lower case.
		///
		
		std::string to_lower(std::string const &str) const;

		///
		/// Convert string in current locale representation (utf8 or other encoding)
		/// to title case.
		///
		std::string to_title(std::string const &str) const;

		///
		/// Perform Unicode normalization of the string in current locale representation
		/// (utf8 or other encoding). Such conversion may be meaningless for non-Unicode locale.
		///
		/// Note: if CppCMS is compiled without support of ICU this function does nothing.
		///

		std::string to_normal(std::string const &str,norm_type how = norm_default) const;

		#ifdef HAVE_STD_WSTRING
		///
		/// Convert wide character string to upper case 
		///
		std::wstring to_upper(std::wstring const &str) const;
		///
		/// Convert wide character string to lower case 
		///
		std::wstring to_lower(std::wstring const &str) const;
		///
		/// Convert wide character string to title case 
		///
		std::wstring to_title(std::wstring const &str) const;
		///
		/// Perform Unicode normalization of the wide character string
		///
		/// Note: if CppCMS is compiled without support of ICU this function does nothing.
		///
		std::wstring to_normal(std::wstring const &str,norm_type how = norm_default) const;
		#endif

		#ifdef HAVE_CPP0X_UXSTRING
		///
		/// Convert utf16 string to upper case 
		///
		std::u16string to_upper(std::u16string const &str) const;
		///
		/// Convert utf16 string to lower case 
		///
		std::u16string to_lower(std::u16string const &str) const;
		///
		/// Convert utf16 string to title case 
		///
		std::u16string to_title(std::u16string const &str) const;
		///
		/// Perform Unicode normalization of utf16 string
		///
		/// Note: if CppCMS is compiled without support of ICU this function does nothing.
		///
		std::u16string to_normal(std::u16string const &str,norm_type how = norm_default) const;

		///
		/// Convert utf32 string to upper case 
		///
		std::u32string to_upper(std::u32string const &str) const;
		///
		/// Convert utf32 string to lower case 
		///
		std::u32string to_lower(std::u32string const &str) const;
		///
		/// Convert utf32 string to title case 
		///
		std::u32string to_title(std::u32string const &str) const;
		///
		/// Perform Unicode normalization of utf32 string
		///
		/// Note: if CppCMS is compiled without support of ICU this function does nothing.
		///
		std::u32string to_normal(std::u32string const &str,norm_type how = norm_default) const;
		#endif

	private:
		
		util::hold_ptr<convert_impl> impl_;
			
	};


} // locale
} // cppcms


#endif
